// KeyboardCameraAnimator.h

#ifndef KEYBOARD_CAMERA_ANIMATOR_H
#define KEYBOARD_CAMERA_ANIMATOR_H

#include "CameraAnimator.h"

class KeyboardCameraAnimator : public CameraAnimator
{
public:
	KeyboardCameraAnimator(Camera* camera = NULL);
	virtual ~KeyboardCameraAnimator();

	virtual void update(double elapsed);

	virtual const char* getName() const {return "KeyboardCameraAnimator";}

	virtual void printHelp() const {}
};

#endif // KEYBOARD_CAMERA_ANIMATOR_H
