// DeferredShadingRenderer.cpp

#include "DeferredShadingRenderer.h"

#include "RasterRenderer.h"
#include "utils/GBuffer.h"
#include "utils/GBufferRenderer.h"
#include "utils/ShadowMap.h"
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

#ifdef USE_DEBUG_TEXTURE
#include "../utils/TGALoader.h"
#endif
using namespace std;

// ---------------------------------------------------------------------
DeferredShadingRenderer::DeferredShadingRenderer(uint width, uint height,
												 const vec3& back_color,
												 bool use_shadow_mapping,
												 bool use_visibility_maps,
												 const string& brdf_function)
: Renderer(width, height, back_color),
  raster_renderer(NULL),
  gbuffer(NULL),
  gbuffer_renderer(NULL),
  use_shadow_mapping(use_shadow_mapping),
  use_visibility_maps(use_visibility_maps),
  brdf_function(brdf_function)
#ifdef USE_DEBUG_TEXTURE
  ,id_debug(0)
#endif
{
	// One should not have shadow mapping disabled and visibility maps enabled.
	assert((use_shadow_mapping ? true : !use_visibility_maps));

	raster_renderer = new RasterRenderer(
			width,
			height,
			vec3(0.0),				// back color: ignored
			use_shadow_mapping,		// raster_renderer manages the creation/deletion of shadow maps
									// and associated VBOs/VAOs.
			use_visibility_maps,
			"",						// brdf_function: none (BRDF evaluation is not done here)
			VAO_INDEX_RASTER,
			VAO_INDEX_RASTER_SHADOW,
			GENERAL_PROFILE);

	gbuffer_renderer = new GBufferRenderer(width, height);
}

DeferredShadingRenderer::~DeferredShadingRenderer()
{
	delete gbuffer_renderer;
	delete raster_renderer;
}

// ---------------------------------------------------------------------
// Called when we switch to this renderer
void DeferredShadingRenderer::setup()
{
	raster_renderer->setup();

	gbuffer = new GBuffer(getWidth(), getHeight(), use_visibility_maps);

#ifdef USE_DEBUG_TEXTURE
	TGALoader tga;
	if(tga.loadFile(DEBUG_TEXTURE_FILENAME) == TGA_OK)
	{
		assert(tga.getBpp() == 4);
		id_debug = glutil::createTextureRGBA8(tga.getWidth(), tga.getHeight(), (const GLubyte*)(tga.getData()));
	}
#endif
}

// Called when we switch to another renderer or close the program
void DeferredShadingRenderer::cleanup()
{
#ifdef USE_DEBUG_TEXTURE
	if(id_debug != 0)
		glDeleteTextures(1, &id_debug);
#endif

	delete gbuffer;
	gbuffer = NULL;

	raster_renderer->cleanup();
}

// ---------------------------------------------------------------------
// Called when we change the scene
// Setup the scene: sort objects by programs and load shaders
void DeferredShadingRenderer::loadSceneArray(Scene* scene)
{
	// Get some pointers/values
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	uint nb_lights = elements->getNbLights();

	// ---------------------------------------
	// Create a list of additional preprocessor symbols for rendering to the GBuffer:
	Preprocessor::SymbolList preproc_syms;
	preproc_syms.push_back(PreprocSym("_RENDER_TO_GBUFFER_"));

	// Load the scene:
	raster_renderer->loadSceneArrayExt(scene, preproc_syms);

	// ---------------------------------------
	// Setup the list of preprocessor symbols used with the GBufferRenderer
	// (rendering the full-screen quad):
	preproc_syms.clear();
	preproc_syms.push_back(PreprocSym("_NB_LIGHTS_", nb_lights));
	if(use_shadow_mapping)
		preproc_syms.push_back(PreprocSym("_SHADOW_MAPPING_", ""));
	if(use_visibility_maps)
		preproc_syms.push_back(PreprocSym("_VISIBILITY_MAPS_", ""));
	preproc_syms.push_back(PreprocSym("_BRDF_FUNCTION_", brdf_function));

#ifdef USE_DEBUG_TEXTURE
	preproc_syms.push_back(PreprocSym("_DEBUG_TEXTURE_", ""));
#endif

	// Setup the GBufferRenderer:
	gbuffer_renderer->load(preproc_syms);
}

// ---------------------------------------------------------------------
// Unload the previous scene
void DeferredShadingRenderer::unloadSceneArray(Scene* scene)
{
	gbuffer_renderer->unload();

	raster_renderer->unloadSceneArray(scene);
}

// ---------------------------------------------------------------------
// Render one frame
void DeferredShadingRenderer::renderArray(Scene* scene)
{
	// Get some pointers/values:
	Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();
	uint texunit = 0;
	glutil::GPUProgram* program = gbuffer_renderer->getProgram();

	// Render the scene's data to the GBuffer:
	gbuffer->render(scene, raster_renderer);

	// Draw a full screen quad while evaluating the GBuffer:
	// - bind what is necessary:
	gbuffer_renderer->bind(gbuffer,
						   camera,
						   lights,
						   nb_lights,
						   use_shadow_mapping && !use_visibility_maps,	// bind_shadow_maps
						   getBackColor(),
						   &texunit);	// first_valid_texunit

	// - bind our additional stuff:
	// => Visibility maps:
	if(use_visibility_maps)
	{
		assert(use_shadow_mapping);
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexVisibility());
		program->sendUniform("visibility_map", GLint(texunit));
		texunit++;
	}

	// => Debug texture:
#ifdef USE_DEBUG_TEXTURE
	glActiveTexture(GL_TEXTURE0 + texunit);
	glBindTexture(GL_TEXTURE_2D, id_debug);
	program->sendUniform("tex_debug", GLint(texunit));
	texunit++;
#endif

	// - draw the quad:
	gbuffer_renderer->render();
}

// ---------------------------------------------------------------------
// 2D debug drawing:
void DeferredShadingRenderer::debugDraw2D(Scene* scene)
{
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	// Display the GBuffer:
	uint w = gbuffer->getWidth();
	uint h = gbuffer->getHeight();

	glutil::displayTextureRect(gbuffer->getTexPositions(), 0, 0, w, h);
	glutil::displayTextureRect(gbuffer->getTexNormals(),   1, 0, w, h);
	glutil::displayTextureRect(gbuffer->getTexDiffuse(),   2, 0, w, h);
	glutil::displayTextureRect(gbuffer->getTexSpecular(),  3, 0, w, h);

	if(use_visibility_maps)
		glutil::displayTextureRect(gbuffer->getTexVisibility(), 4, 0, w, h);
}

// ---------------------------------------------------------------------
// 3D debug drawing:
void DeferredShadingRenderer::debugDraw3D(Scene* scene)
{
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());

	const mat4& proj_matrix = scene->getCamera()->computeProjectionMatrix();
	const mat4& view_matrix = scene->getCamera()->computeViewMatrix();

	// Draw the frustums of the lights:
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	{
		glutil::Disable<GL_DEPTH_TEST> depth_test_state;
		for(uint i=0 ; i < nb_lights ; i++)
		{
			Light* l = lights[i];
			l->debugDrawFrustum(proj_matrix, view_matrix, vec3(1.0, 0.0, 0.0), false);
		}
	}
}
