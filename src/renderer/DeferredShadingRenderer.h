// DeferredShadingRenderer.h

#ifndef DEFERRED_SHADING_RENDERER_H
#define DEFERRED_SHADING_RENDERER_H

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

class DeferredShadingRenderer : public Renderer
{
private:
	RasterRenderer* raster_renderer;
	GBuffer* gbuffer;
	GBufferRenderer* gbuffer_renderer;

	bool use_shadow_mapping;	// NB: current design only allows to set the usage of shadow mapping
								// at creation/deletion of the renderer.

	bool use_visibility_maps;	// If we use the visibility maps, shadow mapping is managed in the raster
								// renderer. Otherwise, we manage the shadow maps and their rendering in
								// this renderer.
	std::string brdf_function;

#ifdef USE_DEBUG_TEXTURE
	GLuint id_debug;
#endif

public:
	DeferredShadingRenderer(uint width, uint height,
							const vec3& back_color,
							bool use_shadow_mapping,
							bool use_visibility_maps,
							const std::string& brdf_function);
	virtual ~DeferredShadingRenderer();

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
	virtual const char* getName() const {return "DeferredShadingRenderer";}

	// Getters:
	GBuffer* getGBuffer() {return gbuffer;}
	const GBuffer* getGBuffer() const {return gbuffer;}
};

#endif // DEFERRED_SHADING_RENDERER_H
