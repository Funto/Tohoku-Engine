// Object.cpp

#include "Object.h"
#include "Material.h"
#include <cstdlib>

Object::Object()
: Element(), material(NULL)
{
}

Object::~Object()
{
	delete material;
}

// RTTI :
const char* Object::getTypeStr() const
{
	Type t = getType();

	switch(t)
	{
	case MESH:
		return "MESH";
	case SPHERE:
		return "SPHERE";
	default:
		return "???";
	}
}

// Material
void Object::setMaterial(Material* material)
{
	delete this->material;
	this->material = material;
}

Material* Object::getMaterial()
{
	return material;
}

const Material* Object::getMaterial() const
{
	return material;
}
