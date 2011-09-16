// Camera.h

#ifndef CAMERA_H
#define CAMERA_H

#include "../Common.h"

class Camera
{
private:
	// Position and orientation
	vec3 position;
	mat3 orientation;

	// Projection values :
	float fovy;		// Angle in degrees
	float aspect;
	float z_near;
	float z_far;
public:
	Camera();
	virtual ~Camera();

	// Position
	void setPosition(const vec3& pos);
	const vec3& getPosition() const;

	// Orientation
	void setOrientation(const mat3& orientation);
	const mat3& getOrientation() const;

	// Projection values
	void setProjection(float fovy, float aspect, float z_near, float z_far);
	float getFOVY() const;
	float getAspect() const;
	float getZNear() const;
	float getZFar() const;

	mat4 computeViewMatrix() const;
	mat4 computeProjectionMatrix() const;
};

#endif // CAMERA_H
