// CameraAnimator.cpp

#include "CameraAnimator.h"
#include "../scene/Camera.h"

CameraAnimator::CameraAnimator(Camera* camera)
: camera(NULL)
{
	setCamera(camera);
}

CameraAnimator::~CameraAnimator()
{
}

// Camera pointer
void CameraAnimator::setCamera(Camera* camera)
{
	// Reset the original transformation of the previous camera
	reset();

	// Backup the original transformation of the new camera
	if(camera != NULL)
	{
		original_position = camera->getPosition();
		original_orientation = camera->getOrientation();
	}

	this->camera = camera;
}

Camera* CameraAnimator::getCamera() const
{
	return camera;
}

// Reset the camera position:
void CameraAnimator::reset()
{
	if(camera != NULL)
	{
		camera->setOrientation(original_orientation);
		camera->setPosition(original_position);
	}
}

// Key event
void CameraAnimator::onKeyEvent(int key, int action)
{
	// Reset :
	if(key == GLFW_KEY_SPACE)
		reset();
}
