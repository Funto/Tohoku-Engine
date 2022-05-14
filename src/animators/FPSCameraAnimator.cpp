// FPSCameraAnimator.cpp

#include "FPSCameraAnimator.h"
#include "../scene/Camera.h"
#include <cmath>
using namespace std;

#define MOVING_SPEED 2.0f
#define TURNING_SPEED 0.003f

FPSCameraAnimator::FPSCameraAnimator(Camera* camera)
: CameraAnimator(camera),
  button_pressed(false),
  original_mouse_x(0),
  original_mouse_y(0)
{
}

FPSCameraAnimator::~FPSCameraAnimator()
{
}

// Function called when switching to this camera animator
void FPSCameraAnimator::load()
{
	button_pressed = false;
	original_mouse_x = 0;
	original_mouse_y = 0;
	original_orientation = mat3();
}

// Mouse click event handler
void FPSCameraAnimator::onMouseButtonEvent(int button, int action)
{
	Camera* camera = getCamera();

	if(camera == NULL)
		return;

	if(button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if(action == GLFW_PRESS)
	{
		glfwGetCursorPos(GLFWWindow::getInstance()->getWindow(), &original_mouse_x, &original_mouse_y);
		button_pressed = true;
		original_orientation = camera->getOrientation();
	}
	else
		button_pressed = false;
}

// Update
void FPSCameraAnimator::update(double elapsed)
{
	Camera* camera = getCamera();

	float dt = float(elapsed);

	if(camera == NULL)
		return;

	// Get some values:
	vec3 dir = -camera->getOrientation()[2];	// -z in the space of the camera object
	vec3 flat_dir = glm::normalize(vec3(dir.x, dir.y, 0.0));	// project the direction in the xy plane
	vec3 pos = camera->getPosition();

	// Read keyboard state:
	bool up     = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_UP       )  == GLFW_PRESS);
	bool down   = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_DOWN     )  == GLFW_PRESS);
	bool left   = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_LEFT     )  == GLFW_PRESS);
	bool right  = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_RIGHT    )  == GLFW_PRESS);
	bool pgup   = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_PAGE_UP  )  == GLFW_PRESS);
	bool pgdown = (glfwGetKey(GLFWWindow::getInstance()->getWindow(), GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);

	// Move forward / backward
	if(up)
		camera->setPosition(pos + flat_dir*dt*MOVING_SPEED);

	if(down)
		camera->setPosition(pos - flat_dir*dt*MOVING_SPEED);
	pos = camera->getPosition();

	// Straff left/right
	if(left || right)
	{
		vec3 cam_x_dir = camera->getOrientation()[0];
		vec3 straff_dir = glm::normalize(vec3(cam_x_dir.x, cam_x_dir.y, 0.0));
		float factor = left ? -1.0 : 1.0;

		camera->setPosition(pos + factor*straff_dir*dt*MOVING_SPEED);
		pos = camera->getPosition();
	}

	// Move up/down
	if(pgup || pgdown)
	{
		float factor = pgdown ? -1.0 : 1.0;
		camera->setPosition(pos + factor*dt*MOVING_SPEED*vec3(0.0, 0.0, 1.0));
		pos = camera->getPosition();
	}

	// Rotate according to the mouse:
	if(button_pressed)
	{
		double mouse_x=0, mouse_y=0;
		glfwGetCursorPos(GLFWWindow::getInstance()->getWindow(), &mouse_x, &mouse_y);

		float dx = float(original_mouse_x-mouse_x);
		float dy = float(original_mouse_y-mouse_y);

		float angle_x = dy*TURNING_SPEED;
		float angle_y = dx*TURNING_SPEED;

		float cx = cos(angle_x);
		float sx = sin(angle_x);
		float cy = cos(angle_y);
		float sy = sin(angle_y);

		mat3 rotation_y(vec3(cy,  0.0, -sy), vec3(0.0, 1.0, 0.0), vec3(sy,  0.0, cy));
		mat3 rotation_x(vec3(1.0, 0.0, 0.0), vec3(0.0, cx,  sx),  vec3(0.0, -sx, cx));

		mat3 orientation = original_orientation*rotation_y*rotation_x;

		camera->setOrientation(orientation);
	}
}
