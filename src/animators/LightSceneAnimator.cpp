// LightSceneAnimator.cpp

#include "LightSceneAnimator.h"
#include "../scene/Scene.h"
#include "../scene/ArrayElementContainer.h"
#include "../scene/Light.h"
#include "../glm/gtc/matrix_transform.hpp"
#include "../log/Log.h"

#include <iostream>
using namespace std;

#define DRAG_SPEED 0.01f
#define ROTATION_SPEED 0.1f
#define DEFAULT_DEPTH_MODE false

LightSceneAnimator::LightSceneAnimator(Scene* scene)
: SceneAnimator(scene),
  original_light_pos(NULL),
  original_light_orientations(NULL),
  num_light(0),
  mouse_right_pressed(false),
  light_pos_before_drag(),
  original_mouse_x(0),
  original_mouse_y(0),
  depth_mode(DEFAULT_DEPTH_MODE),
  num_rotation_axis(-1),
  prev_orientation(1.0)
{
}

LightSceneAnimator::~LightSceneAnimator()
{
	delete [] original_light_pos;
}

// ---------------------------------------------------------------------
// Reset the initial positions and orientations of the lights
void LightSceneAnimator::reset()
{
	ArrayElementContainer* elements = getElementsIfOk();
	if(!elements)
		return;

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	for(uint i=0 ; i < nb_lights ; i++)
	{
		lights[i]->setPosition(original_light_pos[i]);
		lights[i]->setOrientation(original_light_orientations[i]);
	}

	light_pos_before_drag = original_light_pos[num_light];

	depth_mode = DEFAULT_DEPTH_MODE;
}

// ---------------------------------------------------------------------
void LightSceneAnimator::onSceneChanged()
{
	num_light = 0;
	mouse_right_pressed = false;
	original_mouse_x = 0;
	original_mouse_y = 0;

	cout << getName() << ": current light: " << num_light << endl;

	ArrayElementContainer* elements = getElementsIfOk();
	if(!elements)
		return;

	// Backup the original light positions and orientations:
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	delete [] original_light_pos;
	original_light_pos = new vec3[nb_lights];

	delete [] original_light_orientations;
	original_light_orientations = new mat3[nb_lights];

	for(uint i=0 ; i < nb_lights ; i++)
	{
		original_light_pos[i] = lights[i]->getPosition();
		original_light_orientations[i] = lights[i]->getOrientation();
	}

	depth_mode = DEFAULT_DEPTH_MODE;
}

// ---------------------------------------------------------------------
void LightSceneAnimator::update(double elapsed)
{
	ArrayElementContainer* elements = getElementsIfOk();
	if(!elements)
		return;

	Light** lights = elements->getLights();

	// Translation:
	if(mouse_right_pressed)
	{
		int mouse_x=0, mouse_y=0;
		glfwGetMousePos(&mouse_x, &mouse_y);

		float dx = +float(mouse_x-original_mouse_x);
		float dy = -float(mouse_y-original_mouse_y);

		vec3 translation;
		if(depth_mode)
			translation = DRAG_SPEED * vec3(dx, dy,   0.0f);
		else
			translation = DRAG_SPEED * vec3(dx, 0.0f, dy);

		lights[num_light]->setPosition(light_pos_before_drag + translation);

		//logDebug("light pos: ", lights[num_light]->getPosition());
	}

	// Rotation:
	if(num_rotation_axis != -1)
	{
		int mouse_x=0, mouse_y=0;
		glfwGetMousePos(&mouse_x, &mouse_y);

		float dx = +float(mouse_x-original_mouse_x);

		float angle = ROTATION_SPEED * dx;

		vec3 direction(0.0);
		direction[num_rotation_axis] = 1.0;
		mat4 new_orientation = glm::gtc::matrix_transform::rotate(mat4(prev_orientation), angle, direction);
		lights[num_light]->setOrientation(mat3(new_orientation));

		// Move the mouse to the left side if we went too to the right (and the other way around).
		// We need to adjust original_mouse_x for this as well.
		int w=0, h=0;
		glfwGetWindowSize(&w, &h);
		const int margin=20;
		if(mouse_x > w-1-margin)
		{
			glfwSetMousePos(margin, mouse_y);
			original_mouse_x -= mouse_x;
		}
		else if(mouse_x < margin)
		{
			glfwSetMousePos(w-1-margin, mouse_y);
			original_mouse_x += (w-1-mouse_x);
		}
	}
}

// ---------------------------------------------------------------------
// Called when we change the current scene animator
void LightSceneAnimator::load()
{
}

// ---------------------------------------------------------------------
void LightSceneAnimator::unload()
{
}

// ---------------------------------------------------------------------
// Event receivers
void LightSceneAnimator::onMouseButtonEvent(int button, int action)
{
	ArrayElementContainer* elements = getElementsIfOk();
	if(!elements)
		return;

	Light** lights = elements->getLights();

	// Rotation:
	if(mouse_right_pressed == false)
	{
		// Left click: let the light as it is (rotated) and quit the rotation state
		if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
		{
			prev_orientation = mat3(1.0);
			num_rotation_axis = -1;
		}
		// Right click: cancel the rotation
		else if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
		{
			lights[num_light]->setOrientation(prev_orientation);
			num_rotation_axis = -1;
		}
	}

	// Translation:
	if(num_rotation_axis == -1)
	{
		if(button != GLFW_MOUSE_BUTTON_RIGHT)
			return;

		mouse_right_pressed = (action == GLFW_PRESS);

		// Record original mouse position and original light position
		glfwGetMousePos(&original_mouse_x, &original_mouse_y);
		light_pos_before_drag = lights[num_light]->getPosition();
	}
}

// ---------------------------------------------------------------------
// Key event: change the number of the current light
static string nextValue(uint* pval, uint nb_values);
static string prevValue(uint* pval, uint nb_values);

void LightSceneAnimator::onKeyEvent(int key, int action)
{
	SceneAnimator::onKeyEvent(key, action);

	ArrayElementContainer* elements = getElementsIfOk();
	if(!elements)
		return;

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	if(action != GLFW_RELEASE)
		return;

	// Reset :
	if(key == GLFW_KEY_SPACE)
		reset();

	if(key == GLFW_KEY_LSHIFT)
	{
		depth_mode = !depth_mode;
		if(depth_mode)
			cout << "Now moving on the (x, y) plane" << endl;
		else
			cout << "Now moving on the (x, z) plane" << endl;
	}

	string msg = "";

	// We only change the current light or rotate it if nothing is currently
	// happening.
	if(!mouse_right_pressed && num_rotation_axis == -1)
	{
		if(key == 'N')
		{
			msg = nextValue(&num_light, nb_lights);
			cout << msg << "light: " << num_light << endl;
		}
		else if(key == 'P')
		{
			msg = prevValue(&num_light, nb_lights);
			cout << msg << "light: " << num_light << endl;
		}
		else if(key == 'X' || key == 'Y' || key == 'Z')
		{
			cout << "Rotating the light on the " << char(key) << " axis - left click to end, right click to cancel" << endl;
			prev_orientation = lights[num_light]->getOrientation();

			// Record original mouse position and original light position
			glfwGetMousePos(&original_mouse_x, &original_mouse_y);

			num_rotation_axis = int(key-'X');
		}
	}
}

static string nextValue(uint* pval, uint nb_values)
{
	if(*pval == nb_values-1)
		*pval = 0;
	else
		(*pval)++;

	return "Changed to next ";
}

static string prevValue(uint* pval, uint nb_values)
{
	if(*pval == 0)
		*pval = nb_values-1;
	else
		(*pval)--;

	return "Changed to previous ";
}

// ---------------------------------------------------------------------
void LightSceneAnimator::printHelp() const
{
	ArrayElementContainer* elements = getElementsIfOk();
	if(!elements)
		return;

	cout << "--------------" << endl;

	cout << "LightSceneAnimator:" << endl;
	cout << "current light: " << num_light << endl;
	cout << "number of lights: " << elements->getNbLights() << endl;
	cout << "Left click: move the current light" << endl;
	cout << "Shift: change the current axis" << endl;
	cout << "N: next light" << endl;
	cout << "P: previous light" << endl;
	cout << "X, Y, Z: turn the light around the X, Y or Z axis (left click to end, right click to cancel)" << endl;
}

// ---------------------------------------------------------------------
// Check if we are ok and return the elements if it's the case, NULL otherwise
ArrayElementContainer* LightSceneAnimator::getElementsIfOk() const
{
	Scene* scene = getScene();
	if(!scene)
		return NULL;

	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return NULL;

	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	if(elements->getNbLights() == 0)
		return NULL;

	return elements;
}
