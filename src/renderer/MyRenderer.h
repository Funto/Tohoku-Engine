// MyRenderer.h

#ifndef MY_RENDERER_H
#define MY_RENDERER_H

#include "Renderer.h"
#include "../Common.h"
#include <list>
#include <string>

class Camera;
class ArrayElementContainer;
class RasterRenderer;
class MultiLayerRenderer;
class GLRaytracer;
class PhotonVolumesRenderer;

class MyRenderer : public Renderer
{
private:
	RasterRenderer* raster_renderer_no_shadows;	// used for rendering the light GBuffers
	MultiLayerRenderer* multi_layer_renderer;
	GLRaytracer* gl_raytracer;
	PhotonVolumesRenderer* photon_volumes_renderer;

	bool use_shadow_mapping;	// NB: current design only allows to set the usage of shadow mapping
								// at creation/deletion of the renderer.
	bool use_visibility_maps;
	std::string brdf_function;
	uint nb_back_layers;

	bool debug_original_size;	// for debugging: we display the intersection map at its original size

public:
	MyRenderer(uint width, uint height,
			   const vec3& back_color,
			   bool use_shadow_mapping,
			   bool use_visibility_maps,
			   const std::string& brdf_function,
			   const std::string& kernel_filename,
			   uint nb_back_layers);
	virtual ~MyRenderer();

	// Called when we switch to this renderer
	virtual void setup();

	// Called when we switch to another renderer or close the program
	virtual void cleanup();

	// Called when we change the scene
	virtual void loadSceneArray(Scene* scene);
	virtual void unloadSceneArray(Scene* scene);

	// Render one frame
	virtual void renderArray(Scene* scene);

	// Debug drawing
	virtual void debugDraw2D(Scene* scene);
	virtual void debugDraw3D(Scene* scene);

	// Get the renderer's name
	virtual const char* getName() const {return "MyRenderer";}

	// Implementation of KeyEventReceiver:
	virtual void onKeyEvent(int key, int action);
};

#endif // MY_RENDERER_H
