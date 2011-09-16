// Object.h

#ifndef OBJECT_H
#define OBJECT_H

#include "Element.h"

class Material;

class Object : public Element
{
private:
	Material* material;

public:
	// RTTI :
	enum Type
	{
		MESH,
		SPHERE
	};

	virtual Type getType() const = 0;
	const char* getTypeStr() const;

public:
	Object();
	virtual ~Object();

	void setMaterial(Material* material);
	Material* getMaterial();
	const Material* getMaterial() const;
};

#endif // OBJECT_H
