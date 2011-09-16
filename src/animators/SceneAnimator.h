// SceneAnimator.h

#ifndef SCENE_ANIMATOR_H
#define SCENE_ANIMATOR_H

#include "../gui/GLFWWindow.h"
#include "../Common.h"

class Scene;

class SceneAnimator : public EventReceiver
{
private:
	Scene* scene;	// NOT owned

protected:
	inline Scene* getScene() {return scene;}

public:
	SceneAnimator(Scene* scene = NULL);
	virtual ~SceneAnimator();

	void setScene(Scene* scene);
	Scene* getScene() const;
	virtual void onSceneChanged() {}	// Function to be called after the scene has been changed.
										// This includes the case of constructing a new SceneAnimator.

	virtual void reset() {}

	virtual void update(double elapsed) = 0;
	virtual void onKeyEvent(int key, int action);

	// Those functions are called when switching the current scene animator
	virtual void load() {}
	virtual void unload() {}

	virtual const char* getName() const = 0;

	virtual void printHelp() const = 0;
};

#endif // SCENE_ANIMATOR_H
