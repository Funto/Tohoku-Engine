// MeshObject.cpp

#include "MeshObject.h"
#include "Geometry.h"
#include <cstdlib>

MeshObject::MeshObject()
: Object(), geometry(NULL)
{
}

MeshObject::~MeshObject()
{
	delete this->geometry;
}

// Geometry :
void MeshObject::setGeometry(Geometry* geo)
{
	delete this->geometry;
	this->geometry = geo;
}

Geometry* MeshObject::getGeometry()
{
	return geometry;
}

const Geometry* MeshObject::getGeometry() const
{
	return geometry;
}

// RTTI :
Object::Type MeshObject::getType() const
{
	return Object::MESH;
}
