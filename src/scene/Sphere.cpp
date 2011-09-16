// Sphere.cpp

#include "Sphere.h"

Sphere::Sphere()
: Object()
{
}

Sphere::~Sphere()
{
}

// Radius :
void Sphere::setRadius(float radius)
{
	this->radius = radius;
}

float Sphere::getRadius() const
{
	return radius;
}

// RTTI :
Object::Type Sphere::getType() const
{
	return Object::SPHERE;
}
