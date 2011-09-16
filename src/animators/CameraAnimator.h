// CameraAnimator.h

#ifndef CAMERA_ANIMATOR_H
#define CAMERA_ANIMATOR_H

#include "../gui/GLFWWindow.h"
#include "../Common.h"

class Camera;

class CameraAnimator : public EventReceiver
{
private:
	Camera* camera;	// NOT owned

	// Backup transformation of the camera
	mat3 original_orientation;
	vec3 original_position;

protected:
	inline const mat3& getOriginalOrientation() const {return original_orientation;}
	inline const vec3& getOriginalPosition()    const {return original_position;}
	inline Camera* getCamera() {return camera;}

public:
	CameraAnimator(Camera* camera = NULL);
	virtual ~CameraAnimator();

	void setCamera(Camera* camera);
	Camera* getCamera() const;

	void reset();

	virtual void update(double elapsed) = 0;
	virtual void onKeyEvent(int key, int action);

	// Those functions are called when switching the current camera animator
	virtual void load() {}
	virtual void unload() {}

	virtual const char* getName() const = 0;

	virtual void printHelp() const = 0;
};

#endif // CAMERA_ANIMATOR_H
