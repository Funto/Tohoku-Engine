// Renderer.h

#ifndef RENDERER_H
#define RENDERER_H

#include "../Common.h"
#include "../gui/GLFWWindow.h"

class Scene;

class Renderer : public EventReceiver
{
private:
	uint width;
	uint height;
	vec3 back_color;

public:
	Renderer(uint width, uint height, const vec3& back_color);
	virtual ~Renderer();

	// Called when we switch to this renderer
	virtual void setup() = 0;

	// Called when we switch to another renderer or close the program
	virtual void cleanup() = 0;

	// Called when we change the scene
	void loadScene(Scene* scene);
	virtual void loadSceneArray(Scene* scene);
	virtual void loadSceneOctree(Scene* scene);
	virtual void loadSceneBVH(Scene* scene);
	virtual void loadSceneKdTree(Scene* scene);

	void unloadScene(Scene* scene);
	virtual void unloadSceneArray(Scene* scene);
	virtual void unloadSceneOctree(Scene* scene);
	virtual void unloadSceneBVH(Scene* scene);
	virtual void unloadSceneKdTree(Scene* scene);

	// Render a frame
	// render() redirects to the correct render[Array|Octree|...]() function, and keeps track
	// of the last rendered scene so as to call loadScene() / unloadScene() when necessary.
	void render(Scene* scene);
	virtual void renderArray(Scene* scene);
	virtual void renderOctree(Scene* scene);
	virtual void renderBVH(Scene* scene);
	virtual void renderKdTree(Scene* scene);

	// Debug drawing for the renderers
	virtual void debugDraw2D(Scene* scene);
	virtual void debugDraw3D(Scene* scene);

	// Get the renderer's name
	virtual const char* getName() const = 0;

	// Background color
	inline void setBackColor(const vec3& color) {this->back_color = color;}
	inline const vec3& getBackColor() const {return back_color;}

	// Dimensions of the screen
	inline uint getWidth() const {return width;}
	inline uint getHeight() const {return height;}

	// Implementation of KeyEventReceiver:
	virtual void onKeyEvent(int key, int action)
	{
	}
};

#endif // RENDERER_H
