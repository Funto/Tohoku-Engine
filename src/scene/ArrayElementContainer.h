// ArrayElementContainer.h

#ifndef ARRAY_ELEMENT_CONTAINER_H
#define ARRAY_ELEMENT_CONTAINER_H

#include "ElementContainer.h"
#include "../Common.h"
#include <list>

class Object;
class Light;

class ArrayElementContainer : public ElementContainer
{
private:
	Object** objects;	// storage: first opaque objects, then transparent objects
	uint nb_objects;

	uint nb_opaque_objects;
	uint nb_transparent_objects;

	Light** lights;
	uint nb_lights;

	typedef std::list<Object*> ObjectList;
	typedef std::list<Light*> LightList;

	// Temporary lists used when building the scene.
	// Only used between beginFilling() and endFilling().
	ObjectList building_objects;
	LightList building_lights;

public:
	ArrayElementContainer();
	virtual ~ArrayElementContainer();

	// Functions for managing elements :
	virtual void beginFilling();

	virtual void addObject(Object* object);

	virtual void addLight(Light* light);

	virtual void endFilling();

	virtual void clear();

	// Puts opaque objects in the front of the objects array and
	// transparent objects at the end, without changing the relative order.
	void separateOpaqueTransparentObjects();

	// Sorts objects by program for a given profile
	// and calls separateOpaqueTransparentObjects() in the end.
	void sortObjectsByProgram(uint index_profile);

	// RTTI :
	virtual ElementContainer::Type getType() const {return ElementContainer::ARRAY;}

	// Get access to the internal arrays :
	// - all objects:
	Object** getObjects() const {return objects;}
	uint getNbObjects()   const {return nb_objects;}

	// - opaque objects:
	Object** getOpaqueObjects() const {return objects;}
	uint getNbOpaqueObjects() const   {return nb_opaque_objects;}

	// - transparent objects:
	Object** getTransparentObjects() const {return &objects[nb_opaque_objects];}
	uint getNbTransparentObjects() const   {return nb_transparent_objects;}

	// - lights:
	Light** getLights() const {return lights;}
	uint getNbLights() const  {return nb_lights;}
};

#endif // ARRAY_ELEMENT_CONTAINER_H
