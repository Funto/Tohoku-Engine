// MultiLayerRenderer.cpp

#include "MultiLayerRenderer.h"

#include "RasterRenderer.h"
#include "utils/GBuffer.h"
#include "utils/GBufferRenderer.h"
#include "utils/ShadowMap.h"
#include "utils/TextureBinding.h"
#include "../scene/Scene.h"
#include "../scene/MeshObject.h"
#include "../scene/Light.h"
#include "../scene/Camera.h"
#include "../scene/ArrayElementContainer.h"
#include "../scene/Material.h"
#include "../scene/Geometry.h"
#include "../log/Log.h"
#include "../utils/StdListManip.h"
#include <sstream>

using namespace std;

#define DEBUG_DRAW_GBUFFERS

// ---------------------------------------------------------------------
MultiLayerRenderer::MultiLayerRenderer(	uint width, uint height,
										const vec3& back_color,
										bool use_shadow_mapping,
										const string& brdf_function,
										uint nb_back_gbuffers)
: Renderer(width, height, back_color),

  front_gbuffer(NULL),
  back_gbuffers(NULL),
  nb_back_gbuffers(nb_back_gbuffers),

  id_fbo(0),
  id_color(0),
  id_pixels_done(0),
  id_rb_depth(0),

  raster_renderer(NULL),
  raster_renderer_back_layers(NULL),

  gbuffer_renderer(NULL),
  gbuffer_renderer_back_layers(NULL),

  use_shadow_mapping(use_shadow_mapping),
  brdf_function(brdf_function)
{
	// Create renderers:
	// - first part (generating the depth layers):
	raster_renderer = new RasterRenderer(
			width,
			height,
			vec3(0.0),				// back color: ignored
			false,					// shadow mapping is managed directly by this renderer, not by the
									// RasterRenderer.
			false,					// use_visibility_maps: not supported
			"",						// brdf_function: none (BRDF evaluation is not done here)
			VAO_INDEX_RASTER,
			VAO_INDEX_RASTER_SHADOW,
			GENERAL_PROFILE);

	raster_renderer_back_layers = new RasterRenderer(
			width,
			height,
			vec3(0.0),				// back color: ignored
			false,					// shadow mapping is managed directly by this renderer, not by the
									// RasterRenderer.
			false,					// use_visibility_maps: not supported
			"",						// brdf_function: none (BRDF evaluation is not done here)
			VAO_INDEX_RASTER,
			VAO_INDEX_RASTER_SHADOW,
			GENERAL_PROFILE_DEPTH_PEELING);

	// - second part (rendering to the screen):
	gbuffer_renderer = new GBufferRenderer(width, height);

	gbuffer_renderer_back_layers = new GBufferRenderer(width, height);
}

MultiLayerRenderer::~MultiLayerRenderer()
{
	delete gbuffer_renderer_back_layers;
	delete gbuffer_renderer;
	delete raster_renderer_back_layers;
	delete raster_renderer;
}

// ---------------------------------------------------------------------
// Called when we switch to this renderer
void MultiLayerRenderer::setup()
{
	uint width = getWidth();
	uint height = getHeight();

	// Setup the renderers:
	raster_renderer->setup();
	raster_renderer_back_layers->setup();

	// Create the front GBuffer:
	front_gbuffer = new GBuffer(width,
								height,
								false);	// use_visibility_maps: not supported

	// Create the back GBuffers:
	back_gbuffers = new GBuffer*[nb_back_gbuffers];

	for(uint i=0 ; i < nb_back_gbuffers ; i++)
		back_gbuffers[i] = new GBuffer(width, height, false);

	// Create the FBO:
	createFBO();
}

// Called when we switch to another renderer or close the program
void MultiLayerRenderer::cleanup()
{
	deleteFBO();

	for(uint i=0 ; i < nb_back_gbuffers ; i++)
		delete back_gbuffers[i];

	delete [] back_gbuffers;
	back_gbuffers = NULL;

	delete front_gbuffer;
	front_gbuffer = NULL;

	raster_renderer_back_layers->cleanup();
	raster_renderer->cleanup();
}

// ---------------------------------------------------------------------
// Called when we change the scene
// Setup the scene: sort objects by programs and load shaders
void MultiLayerRenderer::loadSceneArray(Scene* scene)
{
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	uint nb_lights = elements->getNbLights();
	Preprocessor::SymbolList preproc_syms;

	// ---------------------------------------
	// - first part of the rendering (RasterRenderer):

	// Common preprocessor symbols:
	preproc_syms.clear();
	preproc_syms.push_back(PreprocSym("_RENDER_TO_GBUFFER_"));

	// => front layer:
	raster_renderer->loadSceneArrayExt(scene, preproc_syms);

	// => back layer:
	raster_renderer_back_layers->loadSceneArrayExt(scene, preproc_syms);

	// ---------------------------------------
	// - second part of the rendering (full-screen quads):

	// Common preprocessor symbols:
	preproc_syms.clear();
	preproc_syms.push_back(PreprocSym("_NB_LIGHTS_", nb_lights));
	if(use_shadow_mapping)
		preproc_syms.push_back(PreprocSym("_SHADOW_MAPPING_"));
	preproc_syms.push_back(PreprocSym("_BRDF_FUNCTION_", brdf_function));
	preproc_syms.push_back(PreprocSym("_MARK_PIXELS_DONE_"));

	// => front layer:
	gbuffer_renderer->load(preproc_syms);

	// => back layers:
	preproc_syms.push_back(PreprocSym("_TEST_PIXELS_DONE_"));
	gbuffer_renderer_back_layers->load(preproc_syms);

	// ---------------------------------------
	// Shadow mapping:
	if(use_shadow_mapping)
		ShadowMap::loadScene(scene, VAO_INDEX_RASTER_SHADOW);
}

// ---------------------------------------------------------------------
// Unload the previous scene
void MultiLayerRenderer::unloadSceneArray(Scene* scene)
{
	if(use_shadow_mapping)
		ShadowMap::unloadScene(scene, VAO_INDEX_RASTER_SHADOW);

	gbuffer_renderer_back_layers->unload();
	gbuffer_renderer->unload();

	raster_renderer_back_layers->unloadSceneArray(scene);
	raster_renderer->unloadSceneArray(scene);
}

// ---------------------------------------------------------------------
// Render one frame
void MultiLayerRenderer::renderArray(Scene* scene)
{
	// Get some pointers/values:
	Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();
	uint texunit = 0;

	uint w = getWidth();
	uint h = getHeight();
    
    GL_CHECK();

	// ------------------------ Render the shadow maps -------------------------
	if(use_shadow_mapping)
		ShadowMap::renderShadowMaps(scene, VAO_INDEX_RASTER_SHADOW);
    
    GL_CHECK();

	// --------- First part: render the scene's data to the GBuffers -----------
	// NB: we alternate between backface culling and front face culling
	{
		// - front GBuffer:
		glCullFace(GL_BACK);
		front_gbuffer->render(scene, raster_renderer);

		// - back GBuffers:
		GBuffer* prev_gbuffer = front_gbuffer;

		for(uint i=0 ; i < nb_back_gbuffers ; i++)
		{
			// Alternate the face culling:
			glCullFace(i % 2 == 0 ? GL_FRONT : GL_BACK);

			// Texture binding for the previous depth layer:
			TextureBinding prev_depth_layer_tex_binding("tex_prev_depth_layer",
														prev_gbuffer->getTexDepth(),
														GL_TEXTURE_RECTANGLE);
			prev_depth_layer_tex_binding.parameters.push_back(
					TextureBinding::Parameter(GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE));
			prev_depth_layer_tex_binding.parameters.push_back(
					TextureBinding::Parameter(GL_TEXTURE_COMPARE_FUNC, GL_GREATER));

			// Render the scene to the current GBuffer:
			back_gbuffers[i]->render(scene, raster_renderer_back_layers, &prev_depth_layer_tex_binding, 1);

			// Update the pointer to the previous GBuffer:
			prev_gbuffer = back_gbuffers[i];
		}
	}
    
    GL_CHECK();

	// ----- Second part: render and composite the GBuffers to the FBO ------
	{
		glutil::BindFramebuffer fbo_binding(id_fbo);

		// Draw a full-screen quad while evaluating the front GBuffer:
		// - bind what is necessary:
		gbuffer_renderer->bind(front_gbuffer,
							   camera,
							   lights,
							   nb_lights,
							   use_shadow_mapping,	// bind_shadow_maps
							   getBackColor(),
							   &texunit);	// first_valid_texunit

		// - bind our additional stuff:
		// => none

		// - draw the quad:
		gbuffer_renderer->render();

		// Draw the back GBuffers:
		// - setup blending:
		glEnable(GL_BLEND);
		//glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_DST_ALPHA);
		glBlendFunc(GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);

		for(uint i=0 ; i < nb_back_gbuffers ; i++)
		{
			GBuffer* gbuffer = back_gbuffers[i];

			// - bind what is necessary:
			gbuffer_renderer_back_layers->bind(gbuffer,
											   camera,
											   lights,
											   nb_lights,
											   use_shadow_mapping,
											   getBackColor(),
											   &texunit);

			// - bind our additional stuff:
			glActiveTexture(GL_TEXTURE0 + texunit);
			glBindTexture(GL_TEXTURE_RECTANGLE, id_pixels_done);
			gbuffer_renderer_back_layers->getProgram()->sendUniform("tex_pixels_done", GLint(texunit));

			// - draw the quad:
			gbuffer_renderer->render();
		}

		glDisable(GL_BLEND);
	}
    
    GL_CHECK();

	// ----- Finally: blitting ------
	// - blit the FBO's color buffer to the screen's color buffer:
	glBindFramebuffer(GL_READ_FRAMEBUFFER, id_fbo);
    GL_CHECK();
	glBlitFramebuffer(0, 0, w, h,
					  0, 0, w, h,
					  GL_COLOR_BUFFER_BIT, GL_NEAREST);
    GL_CHECK();

	// - blit the front GBuffer's depth buffer to the screen's depth buffer:
	/*glBindFramebuffer(GL_READ_FRAMEBUFFER, front_gbuffer->getFBO());
    GL_CHECK();
	glBlitFramebuffer(0, 0, w, h,
					  0, 0, w, h,
					  GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT, GL_NEAREST);
    GL_CHECK();*/

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    GL_CHECK();
}

// ---------------------------------------------------------------------
// 2D debug drawing:
void MultiLayerRenderer::debugDraw2D(Scene* scene)
{
	// Screen position management:
	uint x = 0;
	uint y = 0;
	uint position = 0;

	#define NEXT_POS() \
		position++;	\
		x = position % DEBUG_RECT_FACTOR;	\
		y = position / DEBUG_RECT_FACTOR

	uint w = front_gbuffer->getWidth();
	uint h = front_gbuffer->getHeight();

	// Test if the container is an array container:
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	// FBO for compositing the final image:
	glutil::displayTextureRect(id_color,       x, y, w, h);	NEXT_POS();
	glutil::displayTextureRect(id_pixels_done, x, y, w, h);	NEXT_POS();

	// Draw the GBuffers:
#ifdef DEBUG_DRAW_GBUFFERS
	GBuffer* current_gbuffer = front_gbuffer;
	for(uint i=0 ; i < nb_back_gbuffers+1 ; i++)
	{
		glutil::displayTextureRect(current_gbuffer->getTexPositions(), x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(current_gbuffer->getTexNormals(),   x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(current_gbuffer->getTexDiffuse(),   x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(current_gbuffer->getTexSpecular(),  x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(current_gbuffer->getTexDepth(),     x, y, w, h); NEXT_POS();

		current_gbuffer = back_gbuffers[i];
	}
#endif
}

// ---------------------------------------------------------------------
// 3D debug drawing:
void MultiLayerRenderer::debugDraw3D(Scene* scene)
{
	// Test if the container is an array container:
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	// Get some values/pointers:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());

	const mat4& proj_matrix = scene->getCamera()->computeProjectionMatrix();
	const mat4& view_matrix = scene->getCamera()->computeViewMatrix();

	// Draw the frustums of the lights:
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	{
		glutil::Enable<GL_DEPTH_TEST> depth_test_state;
		for(uint i=0 ; i < nb_lights ; i++)
		{
			Light* l = lights[i];
			l->debugDrawFrustum(proj_matrix, view_matrix, vec3(1.0, 0.0, 0.0), false);
		}
	}
}

// ---------------------------------------------------------------------
// Create the FBO used for compositing the final image:
void MultiLayerRenderer::createFBO()
{
	uint width = getWidth();
	uint height = getHeight();

	// Setup the textures:
	id_color       = glutil::createTextureRectRGBA8(width, height);
	id_pixels_done = glutil::createTextureRectR8(width, height);
	id_rb_depth    = glutil::createRenderbufferDepth(width, height);

	GL_CHECK();

	// Setup the FBO:
	glGenFramebuffers(1, &id_fbo);
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// - attach the textures:
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_color,       0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, id_pixels_done, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, id_rb_depth);

	GL_CHECK();

	// - specify the draw buffers :
	static const GLenum draw_buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
	};
	glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);

	// Check the FBO:
	GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
		logSuccess("FBO creation");
	else
		logError("FBO not complete");
}

void MultiLayerRenderer::deleteFBO()
{
	assert(id_fbo != 0);
	glDeleteFramebuffers(1, &id_fbo);
	id_fbo = 0;

	assert(id_rb_depth != 0);
	glDeleteRenderbuffers(1, &id_rb_depth);
	id_rb_depth = 0;

	assert(id_pixels_done != 0);
	glDeleteTextures(1, &id_pixels_done);
	id_pixels_done = 0;

	assert(id_color != 0);
	glDeleteTextures(1, &id_color);
	id_color = 0;
}
