// MyRenderer2.cpp

#include "MyRenderer2.h"

#include "../CommonIndices.h"
#include "RasterRenderer.h"
#include "DeferredShadingRenderer.h"
#include "utils/BounceMap.h"
#include "utils/PhotonsMap.h"
#include "utils/GBuffer.h"
#include "utils/GLRaytracer.h"
//#include "utils/PhotonsAdvancer.h"
//#include "utils/PhotonVolumesRenderer.h"
#include "utils/MinMaxMipmaps.h"
#include "../scene/Camera.h"
#include "../scene/Scene.h"
#include "../scene/Light.h"
#include "../scene/ArrayElementContainer.h"
#include "../log/Log.h"
using namespace std;

#define DEBUG_DONT_DRAW_PHOTON_VOLUMES
#define NB_ITERATIONS_INDIRECT 1

// ---------------------------------------------------------------------
MyRenderer2::MyRenderer2(uint width, uint height,
					   const vec3& back_color,
					   bool use_shadow_mapping,
					   bool use_visibility_maps,
					   const string& brdf_function,
					   const string& kernel_filename)
: Renderer(width, height, back_color),
  raster_renderer_no_shadows(NULL),
  direct_renderer(NULL),
  //photons_advancer(NULL),
  //photon_volumes_renderer(NULL),	// TODO
  min_max_mipmaps(NULL),
  use_shadow_mapping(use_shadow_mapping),
  use_visibility_maps(use_visibility_maps),
  brdf_function(brdf_function)
{
	// Create the raster renderer we use for the lights' GBuffers:
	raster_renderer_no_shadows = new RasterRenderer(
			512, 512,	// width, height: unused
			vec3(0.0),	// back color
			false,		// use_shadow_mapping
			false,		// use_visibility_maps
			brdf_function,
			VAO_INDEX_RASTER,
			0);	// no VAO index for shadows

	// Create the direct lighting renderer:
	direct_renderer = new DeferredShadingRenderer(
			width, height,
			back_color,
			use_shadow_mapping,
			use_visibility_maps,
			brdf_function);

	// Photons advancer
	//photons_advancer = new PhotonsAdvancer(width, height, 1);

	// Photon volumes renderer:
	// TODO
/*	photon_volumes_renderer = new PhotonVolumesRenderer(width,
														height,
														gl_raytracer->getTargetWidth(),
														gl_raytracer->getTargetHeight(),
														kernel_filename,
														brdf_function);
*/
	// Min-max mipmaps
	min_max_mipmaps = new MinMaxMipmaps(width, height);
}

MyRenderer2::~MyRenderer2()
{
	// Min-max mipmaps
	delete min_max_mipmaps;
	
	// Photon volume renderer:
	//delete photon_volumes_renderer;	// TODO
	
	// Photons advancer:
	//delete photons_advancer;

	// Direct lighting renderer:
	delete direct_renderer;

	// Raster renderer used for the light GBuffers:
	delete raster_renderer_no_shadows;
}

// ---------------------------------------------------------------------
// Called when we switch to this renderer
void MyRenderer2::setup()
{
	// Setup the raster renderer we use for the lights' GBuffers:
	raster_renderer_no_shadows->setup();
	
	// Setup the direct lighting renderer
	direct_renderer->setup();

	// Setup the photons advancer:
	//photons_advancer->setup();
	//gl_raytracer->setup();

	// Setup the photon volumes renderer:
	//photon_volumes_renderer->setup();	// TODO
	
	// Min-max mipmaps
	min_max_mipmaps->setup();
}

// Called when we switch to another renderer or close the program
void MyRenderer2::cleanup()
{
	// Min-max mipmaps
	min_max_mipmaps->cleanup();
	
	// Photon volume renderer:
	//photon_volumes_renderer->cleanup();	// TODO

	// Photons advancer:
	//photons_advancer->cleanup();

	// Direct lighting renderer:
	direct_renderer->cleanup();

	// Raster renderer used for the light GBuffers:
	raster_renderer_no_shadows->cleanup();
}

// ---------------------------------------------------------------------
// Called when we change the scene
// Setup the scene: sort objects by programs and load shaders
void MyRenderer2::loadSceneArray(Scene* scene)
{
	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Load the scene for the direct lighting renderer:
	direct_renderer->loadSceneArray(scene);

	// Add a GBuffer, a BounceMap and a PhotonsMap to each light:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		uint size = l->getBounceMapSize();

		GBuffer* gbuffer = new GBuffer(size, size, false);	// don't use shadow mapping (no visibility map)
		l->setUserData(LIGHT_DATA_GBUFFER, gbuffer);

		BounceMap* bounce_map = new BounceMap(size);
		l->setUserData(LIGHT_DATA_BOUNCE_MAP, bounce_map);
		
		PhotonsMap* photons_map = new PhotonsMap(size);
		l->setUserData(LIGHT_DATA_PHOTONS_MAP, photons_map);
	}
}

// ---------------------------------------------------------------------
// Unload the materials of the previous scene
void MyRenderer2::unloadSceneArray(Scene* scene)
{
	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Remove the user data from the lights
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		l->setUserData(LIGHT_DATA_GBUFFER, NULL);
		l->setUserData(LIGHT_DATA_BOUNCE_MAP, NULL);
		l->setUserData(LIGHT_DATA_PHOTONS_MAP, NULL);
	}

	// Unload the scene for the direct lighting renderer:
	direct_renderer->unloadSceneArray(scene);
}

// ---------------------------------------------------------------------
// Render one frame
void MyRenderer2::renderArray(Scene* scene)
{
	GL_CHECK();

	// Get some pointers/values:
	Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();
	
	// BEGIN DEBUG
	//lights[0]->setPosition(vec3(-0.26, 0.0, 0.62));
	// END DEBUG

	// Direct lighting
	direct_renderer->renderArray(scene);
	
	// Depth min-max mipmapping
	min_max_mipmaps->run(direct_renderer->getGBuffer()->getTexPositions());

	// Compute the eye's projection and view matrix:
	mat4 eye_view = camera->computeViewMatrix();
	mat4 eye_proj = camera->computeProjectionMatrix();

	GL_CHECK();
	
	// For each light:
	// - render to its GBuffer
	// - compute its bounce map
	// - advance the photons
	// - render the photon volumes (indirect illumination):
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		GBuffer* light_gbuffer = (GBuffer*)(l->getUserData(LIGHT_DATA_GBUFFER));
		BounceMap* bounce_map = (BounceMap*)(l->getUserData(LIGHT_DATA_BOUNCE_MAP));
		PhotonsMap* photons_map = (PhotonsMap*)(l->getUserData(LIGHT_DATA_PHOTONS_MAP));

		light_gbuffer->renderFromLight(l, scene, raster_renderer_no_shadows);

		GL_CHECK();
		
		bounce_map->renderFromGBuffer(light_gbuffer);
		GL_CHECK();

		photons_map->run(bounce_map,
		                 light_gbuffer,
		                 direct_renderer->getGBuffer(),
		                 eye_proj,
		                 eye_view,
		                 NB_ITERATIONS_INDIRECT);
		GL_CHECK();

		// TODO
		//GBuffer* gbuffer = direct_renderer->getGBuffer();
		//photons_advancer->run(l, &gbuffer, eye_proj, eye_view, camera->getZNear(), camera->getZFar());
		
//			GBuffer* gbuffer = direct_renderer->getGBuffer();
//			gl_raytracer->run(l, &gbuffer, eye_proj, eye_view, camera->getZNear(), camera->getZFar());

		// TODO
/*
#ifndef DEBUG_DONT_DRAW_PHOTON_VOLUMES
		photon_volumes_renderer->run(eye_view,
									 eye_proj,
									 gl_raytracer,
									 direct_renderer->getGBuffer(),
									 bounce_map->getSize());
#endif
*/
	}
}

// ---------------------------------------------------------------------
// 2D debug drawing:
void MyRenderer2::debugDraw2D(Scene* scene)
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
	
	// Photons maps:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		PhotonsMap* photons_map = (PhotonsMap*)(lights[i]->getUserData(LIGHT_DATA_PHOTONS_MAP));
		uint size = photons_map->getSize();

		glutil::displayTextureRect(photons_map->getTexOutput0(), x, y, size, size); NEXT_POS();
		glutil::displayTextureRect(photons_map->getTexOutput1(), x, y, size, size); NEXT_POS();
	}
	
	// Min-max textures:
	{
		uint w = getWidth() >> 1;
		uint h = getHeight() >> 1;
		uint x_start = x;
		for(uint i=0 ; i < min_max_mipmaps->getNbLayers() ; i++)
		{
			glutil::displayTextureRect(min_max_mipmaps->getMinMaxTex(i), x, y, w, h); //NEXT_POS();
			x = x_start + (i%2);
			if(i%2 == 0)
				y++;
			
			w >>= 1;
			h >>= 1;
		}
	}

#undef NEXT_POS
}

// ---------------------------------------------------------------------
// 3D debug drawing:
void MyRenderer2::debugDraw3D(Scene* scene)
{
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	// Get some pointers/values:
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();
	const mat4& proj_matrix = scene->getCamera()->computeProjectionMatrix();
	const mat4& view_matrix = scene->getCamera()->computeViewMatrix();

	// Draw the frustums of the lights and the photons maps results
	{
		glutil::Disable<GL_DEPTH_TEST> depth_test_state;
		for(uint i=0 ; i < nb_lights ; i++)
		{
			Light* l = lights[i];
			l->debugDrawFrustum(proj_matrix, view_matrix, vec3(1.0, 0.0, 0.0), false);
			
			PhotonsMap* photons_map = (PhotonsMap*)l->getUserData(LIGHT_DATA_PHOTONS_MAP);
			photons_map->debugDraw3D(proj_matrix);
		}
	}
}

// Implementation of KeyEventReceiver:
void MyRenderer2::onKeyEvent(int key, int action)
{
	if(action != GLFW_RELEASE)
		return;
}
