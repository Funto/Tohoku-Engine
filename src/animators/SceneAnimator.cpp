// SceneAnimator.cpp

#include "SceneAnimator.h"
#include "../scene/Camera.h"

SceneAnimator::SceneAnimator(Scene* scene)
: scene(NULL)
{
	setScene(scene);
}

SceneAnimator::~SceneAnimator()
{
}

// Camera pointer
void SceneAnimator::setScene(Scene* scene)
{
	this->scene = scene;
}

Scene* SceneAnimator::getScene() const
{
	return scene;
}

// Key event
void SceneAnimator::onKeyEvent(int key, int action)
{
	// Reset :
	if(key == GLFW_KEY_ENTER)
		reset();
}
