// MultiLayerRenderer.h

#ifndef MULTI_LAYER_RENDERER_H
#define MULTI_LAYER_RENDERER_H

#include "Renderer.h"
#include "../Common.h"
#include "../Debug.h"
#include "../glutil/glutil.h"
#include <list>
#include <string>

class GBuffer;
class GBufferRenderer;
class Light;
class Camera;
class ArrayElementContainer;
class RasterRenderer;

class MultiLayerRenderer : public Renderer
{
private:
	// ------------------------------------------
	// Buffers:
	// - front GBuffer:
	GBuffer* front_gbuffer;

	// - back GBuffers:
	GBuffer** back_gbuffers;
	uint nb_back_gbuffers;

	// ------------------------------------------
	// FBO used for the compositing the final image:
	GLuint id_fbo;

	GLuint id_color;

	// - texture used for avoiding unnecessary computations (shading...) on the back
	// layers in the final rendering part.
	GLuint id_pixels_done;

	GLuint id_rb_depth;

	// ------------------------------------------
	// Renderers:
	// - first part (generating the depth layers):
	RasterRenderer* raster_renderer;	// Associated with GENERAL_PROFILE: used to render the
										// front GBuffer.
	RasterRenderer* raster_renderer_back_layers;	// Associated with GENERAL_PROFILE_DEPTH_PEELING:
													// used to render the next layers.

	// - second part (rendering to the screen):
	GBufferRenderer* gbuffer_renderer;	// Used for rendering the front GBuffer.
										// _MARK_PIXELS_DONE_ is defined.

	GBufferRenderer* gbuffer_renderer_back_layers;	// Used for rendering the next layers.
													// _MARK_PIXELS_DONE_ and _TEST_PIXELS_DONE_ are defined.

	// ------------------------------------------
	// Others:

	bool use_shadow_mapping;	// NB: current design only allows to set the usage of shadow mapping
								// at creation/deletion of the renderer.

	std::string brdf_function;

	// ------------------------------------------
public:
	MultiLayerRenderer(uint width, uint height,
							const vec3& back_color,
							bool use_shadow_mapping,
							const std::string& brdf_function,
							uint nb_back_gbuffers);
	virtual ~MultiLayerRenderer();

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
	virtual const char* getName() const {return "MultiLayerRenderer";}

	// Getters:
	GBuffer* getFrontGBuffer() {return front_gbuffer;}
	const GBuffer* getFrontGBuffer() const {return front_gbuffer;}

	GBuffer** getBackGBuffers() const {return back_gbuffers;}
	uint getNbBackGBuffers() const {return nb_back_gbuffers;}

private:
	void createFBO();
	void deleteFBO();
};

#endif // MULTI_LAYER_RENDERER_H
