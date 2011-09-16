// Sphere.h

#ifndef SPHERE_H
#define SPHERE_H

#include "Object.h"

class Sphere : public Object
{
	float radius;
public:
	Sphere();
	virtual ~Sphere();

	void setRadius(float radius);
	float getRadius() const;

	// RTTI :
	virtual Object::Type getType() const;
};

#endif // SPHERE_H
