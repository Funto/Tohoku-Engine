// Camera.cpp

#include "Camera.h"
#include "../glm/gtc/matrix_transform.hpp"
#include "../glm/gtc/matrix_projection.hpp"

Camera::Camera()
: position(), orientation(), fovy(45.0), aspect(1.0), z_near(1.0), z_far(1000.0)
{
}

Camera::~Camera()
{
}

// Position
void Camera::setPosition(const vec3& pos)
{
	this->position = pos;
}

const vec3& Camera::getPosition() const
{
	return position;
}

// Orientation
void Camera::setOrientation(const mat3& orientation)
{
	this->orientation = orientation;
}

const mat3& Camera::getOrientation() const
{
	return orientation;
}

// Projection values
void Camera::setProjection(float fovy, float aspect, float z_near, float z_far)
{
	this->fovy = fovy;
	this->aspect = aspect;
	this->z_near = z_near;
	this->z_far = z_far;
}

float Camera::getFOVY() const
{
	return fovy;
}

float Camera::getAspect() const
{
	return aspect;
}

float Camera::getZNear() const
{
	return z_near;
}

float Camera::getZFar() const
{
	return z_far;
}

// View matrix
mat4 Camera::computeViewMatrix() const
{
	mat4 m(glm::inverse(orientation));
	return glm::gtc::matrix_transform::translate(m, -position);
}

// Projection matrix
mat4 Camera::computeProjectionMatrix() const
{
	return glm::gtc::matrix_projection::perspective(fovy, aspect, z_near, z_far);
}
