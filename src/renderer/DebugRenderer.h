// DebugRenderer.h

#ifndef DEBUG_RENDERER_H
#define DEBUG_RENDERER_H

#include "Renderer.h"
#include "../Debug.h"
#include "../glutil/glutil.h"
#include <string>

class DebugRenderer : public Renderer
{
private:
	bool see_from_light;
	uint num_light_viewer;

#ifdef USE_DEBUG_TEXTURE
	GLuint id_debug;
#endif

	std::string brdf_function;

public:
	DebugRenderer(uint width, uint height, const vec3& back_color, const std::string& brdf_function);
	virtual ~DebugRenderer();

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

	// Get the renderer's name
	virtual const char* getName() const {return "DebugRenderer";}

	virtual void onKeyEvent(int key, int action);
};

#endif // DEBUG_RENDERER_H
