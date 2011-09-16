// StencilRoutedRenderer.h

#ifndef STENCIL_ROUTED_RENDERER_H
#define STENCIL_ROUTED_RENDERER_H

#include "Renderer.h"
#include "../glutil/GPUProgram.h"

class StencilRoutedRenderer : public Renderer
{
private:
	glutil::GPUProgram* write_fragments_program;
	glutil::GPUProgram* sort_fragments_program;

public:
	StencilRoutedRenderer(uint width, uint height);
	virtual ~StencilRoutedRenderer();

	// Called when we switch to this renderer
	virtual void setup();

	// Called when we switch to another renderer or close the program
	virtual void cleanup();

	// Called when we change the scene
	virtual void loadSceneArray(Scene* scene);
	virtual void unloadSceneArray(Scene* scene);

	// Render a frame
	virtual void renderArray(Scene* scene);

	// Debug drawing for the renderers
	virtual void debugDraw2D(Scene* scene);
	virtual void debugDraw3D(Scene* scene);

	// Get the renderer's name
	virtual const char* getName() const {return "StencilRoutedRenderer";}

private:
	void createPrograms();
};

#endif // STENCIL_ROUTED_RENDERER_H
