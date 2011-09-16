// MeshObject.h

#ifndef MESH_OBJECT_H
#define MESH_OBJECT_H

#include "Object.h"

class Geometry;

class MeshObject : public Object
{
private:
	Geometry* geometry;

public:
	MeshObject();
	virtual ~MeshObject();

	void setGeometry(Geometry* geo);

	Geometry* getGeometry();
	const Geometry* getGeometry() const;

	// RTTI :
	virtual Object::Type getType() const;
};

#endif // MESH_OBJECT_H
