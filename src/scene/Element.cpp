// Element.cpp

#include "Element.h"

Element::Element()
{
}

Element::~Element()
{
}

// Position
void Element::setPosition(const vec3& pos)
{
	this->position = pos;
}

const vec3& Element::getPosition() const
{
	return position;
}

// Orientation
void Element::setOrientation(const mat3& orientation)
{
	this->orientation = orientation;
}

const mat3& Element::getOrientation() const
{
	return orientation;
}

// Name :
void Element::setName(const std::string& name)
{
	this->name = name;
}

const std::string& Element::getName() const
{
	return name;
}
