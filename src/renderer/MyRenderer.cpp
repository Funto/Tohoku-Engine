// MyRenderer.cpp

#include "MyRenderer.h"

#include "../CommonIndices.h"
#include "RasterRenderer.h"
#include "MultiLayerRenderer.h"
#include "utils/BounceMap.h"
#include "utils/GBuffer.h"
#include "utils/GLRaytracer.h"
#include "utils/PhotonVolumesRenderer.h"
#include "../scene/Camera.h"
#include "../scene/Scene.h"
#include "../scene/Light.h"
#include "../scene/ArrayElementContainer.h"
#include "../log/Log.h"
using namespace std;

//#define DEBUG_DONT_DRAW_PHOTON_VOLUMES
#define NB_ITERATIONS_INDIRECT 1

// ---------------------------------------------------------------------
MyRenderer::MyRenderer(uint width, uint height,
					   const vec3& back_color,
					   bool use_shadow_mapping,
					   bool use_visibility_maps,
					   const string& brdf_function,
					   const string& kernel_filename,
					   uint nb_back_layers)
: Renderer(width, height, back_color),
  raster_renderer_no_shadows(NULL),
  multi_layer_renderer(NULL),
  gl_raytracer(NULL),
  photon_volumes_renderer(NULL),
  use_shadow_mapping(use_shadow_mapping),
  use_visibility_maps(use_visibility_maps),
  brdf_function(brdf_function),
  nb_back_layers(nb_back_layers),
  debug_original_size(false)
{
	// Create and the raster renderer we use for the lights' GBuffers:
	raster_renderer_no_shadows = new RasterRenderer(
			512, 512,	// width, height: unused
			vec3(0.0),	// back color
			false,		// use_shadow_mapping
			false,		// use_visibility_maps
			brdf_function,
			VAO_INDEX_RASTER,
			0);	// no VAO index for shadows

	// Create the multi-layer renderer:
	multi_layer_renderer = new MultiLayerRenderer(
			width, height,
			back_color,
			use_shadow_mapping,
			brdf_function,
			nb_back_layers);

	// Raytracer:
	gl_raytracer = new GLRaytracer(width,
								   height,
								   nb_back_layers+1);

	// Photon volumes renderer:
	photon_volumes_renderer = new PhotonVolumesRenderer(width,
														height,
														gl_raytracer->getTargetWidth(),
														gl_raytracer->getTargetHeight(),
														kernel_filename,
														brdf_function);
}

MyRenderer::~MyRenderer()
{
	// Photon volumes renderer:
	delete photon_volumes_renderer;

	// Multi-layer renderer:
	delete multi_layer_renderer;

	// Raster renderer used for the light GBuffers:
	delete raster_renderer_no_shadows;
}

// ---------------------------------------------------------------------
// Called when we switch to this renderer
void MyRenderer::setup()
{
	// Setup the raster renderer we use for the lights' GBuffers:
	raster_renderer_no_shadows->setup();

	// Setup the multi-layer renderer:
	multi_layer_renderer->setup();

	// Setup the GPU raytracer:
	gl_raytracer->setup();

	// Setup the photon volumes renderer:
	photon_volumes_renderer->setup();
}

// Called when we switch to another renderer or close the program
void MyRenderer::cleanup()
{
	// Photon volume renderer:
	photon_volumes_renderer->cleanup();

	// GPU raytracer:
	gl_raytracer->cleanup();

	// Multi-layer renderer:
	multi_layer_renderer->cleanup();

	// Raster renderer used for the light GBuffers:
	raster_renderer_no_shadows->cleanup();
}

// ---------------------------------------------------------------------
// Called when we change the scene
// Setup the scene: sort objects by programs and load shaders
void MyRenderer::loadSceneArray(Scene* scene)
{
	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Load the scene for the multi-layer renderer:
	multi_layer_renderer->loadSceneArray(scene);

	// Add one GBuffer and one BounceMap to each light:
	// For each light, add a GBuffer and a BounceMap to it:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		uint size = l->getBounceMapSize();

		GBuffer* gbuffer = new GBuffer(size, size, false);	// don't use shadow mapping (no visibility map)
		l->setUserData(LIGHT_DATA_GBUFFER, gbuffer);

		BounceMap* bounce_map = new BounceMap(size);
		l->setUserData(LIGHT_DATA_BOUNCE_MAP, bounce_map);
	}
}

// ---------------------------------------------------------------------
// Unload the materials of the previous scene
void MyRenderer::unloadSceneArray(Scene* scene)
{
	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Remove the GBuffers and the bounce maps of the lights:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		l->setUserData(LIGHT_DATA_GBUFFER, NULL);
		l->setUserData(LIGHT_DATA_BOUNCE_MAP, NULL);
	}

	// Unload the scene for the multi-layer renderer:
	multi_layer_renderer->unloadSceneArray(scene);
}

// ---------------------------------------------------------------------
// Render one frame
void MyRenderer::renderArray(Scene* scene)
{
    GL_CHECK();

	// Get some pointers/values:
	Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();
	GBuffer** back_gbuffers = multi_layer_renderer->getBackGBuffers();
	uint nb_back_gbuffers = multi_layer_renderer->getNbBackGBuffers();

	// BEGIN DEBUG
	//lights[0]->setPosition(vec3(-0.26, 0.0, 0.62));
	// END DEBUG

	// Create an array of pointers to the GBuffers representing the scene:
	GBuffer* gbuffers[NB_MAX_DEPTH_LAYERS];
	gbuffers[0] = multi_layer_renderer->getFrontGBuffer();
	for(uint i=0 ; i < nb_back_gbuffers ; i++)
		gbuffers[i+1] = back_gbuffers[i];
    
    GL_CHECK();

	// Do a depth peeling and a deferred shading rendering:
	multi_layer_renderer->renderArray(scene);

	// Compute the eye's projection and view matrix:
	mat4 eye_view = camera->computeViewMatrix();
	mat4 eye_proj = camera->computeProjectionMatrix();
    
    GL_CHECK();

	// For each light:
	// - render to its GBuffer
	// - compute its bounce map
	// - raytrace the photons starting with the bounce map
	// - render the photon volumes (indirect illumination):
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		GBuffer* light_gbuffer = (GBuffer*)(l->getUserData(LIGHT_DATA_GBUFFER));
		BounceMap* bounce_map = (BounceMap*)(l->getUserData(LIGHT_DATA_BOUNCE_MAP));

		light_gbuffer->renderFromLight(l, scene, raster_renderer_no_shadows);
        
        GL_CHECK();

		for(uint j=0 ; j < NB_ITERATIONS_INDIRECT ; j++)
		{
			bounce_map->renderFromGBuffer(light_gbuffer, j);
            
            GL_CHECK();

			gl_raytracer->run(l, gbuffers, eye_proj, eye_view, camera->getZNear(), camera->getZFar());

#ifndef DEBUG_DONT_DRAW_PHOTON_VOLUMES
			photon_volumes_renderer->run(eye_view,
										 eye_proj,
										 gl_raytracer,
										 multi_layer_renderer->getFrontGBuffer(),
										 bounce_map->getSize());
#endif
		}
	}
}

// ---------------------------------------------------------------------
// 2D debug drawing:
void MyRenderer::debugDraw2D(Scene* scene)
{
	uint x = 0;
	uint y = 0;
	uint position = 0;

#define NEXT_POS() \
	position++;	\
	x = position % DEBUG_RECT_FACTOR;	\
	y = position / DEBUG_RECT_FACTOR

	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Intersection map:
	{
		uint w = gl_raytracer->getTargetWidth();
		uint h = gl_raytracer->getTargetHeight();

		GLuint id_debug = gl_raytracer->getDebugTex();
		if(id_debug != 0)
		{
			glutil::displayTextureRect(id_debug, x, y, w, h, debug_original_size); NEXT_POS();
		}
		glutil::displayTextureRect(gl_raytracer->getPositionTex(),  x, y, w, h, debug_original_size); NEXT_POS();
		glutil::displayTextureRect(gl_raytracer->getPowerTex(),     x, y, w, h, debug_original_size); NEXT_POS();
		glutil::displayTextureRect(gl_raytracer->getComingDirTex(), x, y, w, h, debug_original_size); NEXT_POS();
	}

	// Lights GBuffers:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		GBuffer* gbuffer = (GBuffer*)(lights[i]->getUserData(LIGHT_DATA_GBUFFER));
		uint w = gbuffer->getWidth();
		uint h = gbuffer->getHeight();

		glutil::displayTextureRect(gbuffer->getTexPositions(), x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(gbuffer->getTexNormals(),   x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(gbuffer->getTexDiffuse(),   x, y, w, h);	NEXT_POS();
		glutil::displayTextureRect(gbuffer->getTexSpecular(),  x, y, w, h);	NEXT_POS();
	}

	// Bounce maps:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		BounceMap* bounce_map = (BounceMap*)(lights[i]->getUserData(LIGHT_DATA_BOUNCE_MAP));
		uint size = bounce_map->getSize();

		glutil::displayTextureRect(bounce_map->getTexOutput0(), x, y, size, size); NEXT_POS();
		glutil::displayTextureRect(bounce_map->getTexOutput1(), x, y, size, size); NEXT_POS();
	}

#undef NEXT_POS
}

// ---------------------------------------------------------------------
// 3D debug drawing:
void MyRenderer::debugDraw3D(Scene* scene)
{
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();
	const mat4& proj_matrix = scene->getCamera()->computeProjectionMatrix();
	const mat4& view_matrix = scene->getCamera()->computeViewMatrix();

	// Draw the frustums of the lights:
	{
		glutil::Enable<GL_DEPTH_TEST> depth_test_state;
		for(uint i=0 ; i < nb_lights ; i++)
		{
			Light* l = lights[i];
			l->debugDrawFrustum(proj_matrix, view_matrix, vec3(1.0, 0.0, 0.0), false);
		}
	}
}

// Implementation of KeyEventReceiver:
void MyRenderer::onKeyEvent(int key, int action)
{
	if(action != GLFW_RELEASE)
		return;

	if(key == 'F')
		debug_original_size = !debug_original_size;
}
