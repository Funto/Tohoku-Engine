// FPSCameraAnimator.h

#ifndef FPS_CAMERA_ANIMATOR_H
#define FPS_CAMERA_ANIMATOR_H

#include "CameraAnimator.h"

class FPSCameraAnimator : public CameraAnimator
{
private:
	bool button_pressed;
	double original_mouse_x;
	double original_mouse_y;
	mat3 original_orientation;

public:
	FPSCameraAnimator(Camera* camera = NULL);
	virtual ~FPSCameraAnimator();

	virtual void load();
	virtual void onMouseButtonEvent(int button, int action);

	virtual void update(double elapsed);

	virtual const char* getName() const {return "FPSCameraAnimator";}

	virtual void printHelp() const {}
};

#endif // FPS_CAMERA_ANIMATOR_H
