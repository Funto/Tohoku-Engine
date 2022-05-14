// LightSceneAnimator.h

#ifndef LIGHT_SCENE_ANIMATOR_H
#define LIGHT_SCENE_ANIMATOR_H

#include "SceneAnimator.h"
#include "../Common.h"

class ArrayElementContainer;

class LightSceneAnimator : public SceneAnimator
{
private:
	vec3* original_light_pos;
	mat3* original_light_orientations;
	uint num_light;	// Number of the light to move

	bool mouse_right_pressed;
	vec3 light_pos_before_drag;
	double original_mouse_x;
	double original_mouse_y;

	bool depth_mode;

	// Rotation:
	int num_rotation_axis;	// -1: no rotation
							// 0: around x, 1: around y, 2: around z
	mat3 prev_orientation;

public:
	LightSceneAnimator(Scene* scene = NULL);

	virtual ~LightSceneAnimator();

	virtual void update(double elapsed);

	virtual const char* getName() const {return "LightSceneAnimator";}

	virtual void reset();

	virtual void onSceneChanged();

	// Called when we change the current scene animator
	virtual void load();
	virtual void unload();

	// Event receivers
	virtual void onMouseButtonEvent(int button, int action);
	virtual void onKeyEvent(int key, int action);

	virtual void printHelp() const;

private:
	// Check if we are ok and return the elements if it's the case, NULL otherwise
	ArrayElementContainer* getElementsIfOk() const;
};

#endif // LIGHT_SCENE_ANIMATOR_H
