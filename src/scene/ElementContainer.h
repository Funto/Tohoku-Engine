// ElementContainer.h

#ifndef ELEMENT_CONTAINER_H
#define ELEMENT_CONTAINER_H

class Object;
class Light;

class ElementContainer
{
public:
	ElementContainer() {}
	virtual ~ElementContainer() {}

	// Functions for managing elements :
	virtual void beginFilling() = 0;

	virtual void addObject(Object* object) = 0;

	virtual void addLight(Light* light) = 0;

	virtual void endFilling() = 0;

	virtual void clear() = 0;

	// RTTI :
	enum Type
	{
		ARRAY,
		OCTREE,
		BVH,
		KD_TREE
	};

	virtual Type getType() const = 0;

	const char* getTypeStr() const;

	static const char* getTypeStr(Type t);

	// Factory :
	static ElementContainer* create(Type type);
};

#endif // ELEMENT_CONTAINER_H
