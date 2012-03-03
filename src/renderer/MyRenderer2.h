// MyRenderer2.h

#ifndef MY_RENDERER2_H
#define MY_RENDERER2_H

#include "Renderer.h"
#include "../Common.h"
#include <list>
#include <string>

class Camera;
class ArrayElementContainer;
class RasterRenderer;
class DeferredShadingRenderer;
//class PhotonsAdvancer;
class PhotonVolumesRenderer;

class MyRenderer2 : public Renderer
{
private:
	RasterRenderer*				raster_renderer_no_shadows;	// used for rendering the light GBuffers
	DeferredShadingRenderer*	direct_renderer;	// direct lighting
	//PhotonsAdvancer*			photons_advancer;
	//PhotonVolumesRenderer*		photon_volumes_renderer;	// TODO

	bool		use_shadow_mapping;	// NB: current design only allows to set the usage of shadow mapping
									// at creation/deletion of the renderer.
	bool		use_visibility_maps;
	std::string	brdf_function;

public:
	MyRenderer2(uint width, uint height,
			   const vec3& back_color,
			   bool use_shadow_mapping,
			   bool use_visibility_maps,
			   const std::string& brdf_function,
			   const std::string& kernel_filename);
	virtual ~MyRenderer2();

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
	virtual const char* getName() const {return "MyRenderer2";}

	// Implementation of KeyEventReceiver:
	virtual void onKeyEvent(int key, int action);
};

#endif // MY_RENDERER2_H
