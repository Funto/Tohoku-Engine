// KeyboardCameraAnimator.cpp

#include "KeyboardCameraAnimator.h"
#include "../scene/Camera.h"
#include <cmath>
using namespace std;

#define MOVING_SPEED 2.0f
#define TURNING_SPEED 1.0f

KeyboardCameraAnimator::KeyboardCameraAnimator(Camera* camera)
: CameraAnimator(camera)
{
}

KeyboardCameraAnimator::~KeyboardCameraAnimator()
{
}

// Update
void KeyboardCameraAnimator::update(double elapsed)
{
	Camera* camera = getCamera();

	float dt = float(elapsed);

	if(camera == NULL)
		return;

	vec3 dir = -camera->getOrientation()[2];
	vec3 pos = camera->getPosition();

	bool up     = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_UP       )  == GLFW_PRESS);
	bool down   = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_DOWN     )  == GLFW_PRESS);
	bool left   = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_LEFT     )  == GLFW_PRESS);
	bool right  = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_RIGHT    )  == GLFW_PRESS);
	bool pgup   = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_PAGE_UP  )  == GLFW_PRESS);
	bool pgdown = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);

	if(up)
		camera->setPosition(pos + dir*dt*MOVING_SPEED);

	if(down)
		camera->setPosition(pos - dir*dt*MOVING_SPEED);

	if(left || right)
	{
		mat3 orientation = camera->getOrientation();

		float theta = dt*TURNING_SPEED;

		if(right && !left)
			theta = -theta;

		float s = sin(theta);
		float c = cos(theta);

		mat3 rotation(vec3(c, 0.0, -s), vec3(0.0, 1.0, 0.0), vec3(s, 0.0, c));
		camera->setOrientation(orientation*rotation);
	}

	if(pgup || pgdown)
	{
		mat3 orientation = camera->getOrientation();

		float phi = dt*TURNING_SPEED;

		if(pgdown)
			phi = -phi;

		float s = sin(phi);
		float c = cos(phi);

		mat3 rotation(vec3(1.0, 0.0, 0.0), vec3(0.0, c, s), vec3(0.0, -s, c));
		camera->setOrientation(orientation*rotation);
	}
}
