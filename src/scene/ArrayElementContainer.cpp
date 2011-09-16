// ArrayElementContainer.cpp

#include "ArrayElementContainer.h"
#include "profiles/GPUProfile.h"
#include "Object.h"
#include "Light.h"
#include "Material.h"
#include "../scene/GPUProgramManager.h"
#include "../glutil/GPUProgram.h"
#include "../utils/List.h"
#include <cassert>
#include <cstring>
using namespace std;

ArrayElementContainer::ArrayElementContainer()
: ElementContainer(),
  objects(NULL),
  nb_objects(0),
  nb_opaque_objects(0),
  nb_transparent_objects(0),
  lights(NULL),
  nb_lights(0)
{
}

ArrayElementContainer::~ArrayElementContainer()
{
	clear();
}

// ---------------------------------------------------------------------
// Functions for managing elements :
void ArrayElementContainer::beginFilling()
{
	assert(building_objects.size() == 0);
	assert(building_lights.size() == 0);

	clear();
}

void ArrayElementContainer::addObject(Object* object)
{
	assert(objects == NULL);
	assert(nb_objects == 0);
	building_objects.push_back(object);

	const Material* material = object->getMaterial();
	assert(material != NULL);

	Material::Flags flags = material->getFlags();

	if(flags & MATERIAL_OPAQUE)
		nb_opaque_objects++;
	else if(flags & MATERIAL_TRANSPARENT)
		nb_transparent_objects++;
	else
	{
		logError("material \"", material->getFilename(), "\" is neither marked as opaque nor transparent");
		assert(false);
	}
}

void ArrayElementContainer::addLight(Light* light)
{
	assert(lights== NULL);
	assert(nb_lights == 0);
	building_lights.push_back(light);
}

void ArrayElementContainer::endFilling()
{
	// Create the arrays :
	nb_objects = building_objects.size();
	nb_lights = building_lights.size();

	assert(nb_objects == nb_opaque_objects + nb_transparent_objects);

	objects = new Object*[nb_objects];
	lights = new Light*[nb_lights];

	// Copy the building lists to the arrays
	uint i=0;
	ObjectList::iterator it_end_obj = building_objects.end();
	ObjectList::iterator it_obj = building_objects.begin();

	for(it_obj = building_objects.begin(), i=0 ;
		it_obj != it_end_obj ;
		it_obj++, i++)
	{
		objects[i] = *it_obj;
	}

	LightList::iterator it_end_light = building_lights.end();
	LightList::iterator it_light = building_lights.begin();

	for(it_light = building_lights.begin(), i=0 ;
		it_light != it_end_light ;
		it_light++, i++)
	{
		lights[i] = *it_light;
	}

	// Clear the building lists
	building_objects.clear();
	building_lights.clear();

	// Separate opaque and transparent objects:
	separateOpaqueTransparentObjects();
}

void ArrayElementContainer::clear()
{
	// We should not clear between beginFilling() and endFilling()
	assert(building_objects.size() == 0);
	assert(building_lights.size() == 0);

	// Delete objects :
	for(uint i=0 ; i < nb_objects ; i++)
		delete objects[i];

	delete [] objects;
	nb_objects = 0;
	objects = NULL;

	// Delete lights :
	for(uint i=0 ; i < nb_lights ; i++)
		delete lights[i];

	delete [] lights;
	nb_lights = 0;
	lights = NULL;
}

// ---------------------------------------------------------------------
void ArrayElementContainer::separateOpaqueTransparentObjects()
{
	assert(nb_objects == nb_opaque_objects + nb_transparent_objects);
	uint index = 0;

	// Create a copy of the objects array:
	Object** objects_copy = new Object*[nb_objects];
	memcpy(objects_copy, objects, nb_objects*sizeof(Object*));

	// Copy the opaque objects from "objects_copy" to "objects":
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects_copy[i];
		Material* material = obj->getMaterial();
		assert(material != NULL);

		if(material->getFlags() & MATERIAL_OPAQUE)
			objects[index++] = obj;
	}

	// Copy the transparent objects from "objects_copy" to "objects":
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects_copy[i];
		Material* material = obj->getMaterial();
		assert(material != NULL);

		if(material->getFlags() & MATERIAL_TRANSPARENT)
			objects[index++] = obj;
	}

	assert(index == nb_objects);

	delete [] objects_copy;
}

// ---------------------------------------------------------------------
// This structure is used to group together objects which share the
// same GPUProgram in a given profile
struct ObjectsAndProgram
{
	List<Object*> objects;
	const glutil::GPUProgram* program;
};

void ArrayElementContainer::sortObjectsByProgram(uint index_profile)
{
	// Special case of no objects
	if(nb_objects == 0)
		return;

	// ------ First step : store "categories" of objects in a list -------
	List<ObjectsAndProgram> sorted_objects;

	// For each object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects[i];
		const GPUProfile* gpu_profile = (const GPUProfile*)(obj->getMaterial()->getProfile(index_profile));
		const glutil::GPUProgram* program = gpu_profile->getProgram();

		bool found = false;

		// For each ObjectsAndProgram in the temporary list:
		List<ObjectsAndProgram>::Iterator it_end = sorted_objects.end();
		for(List<ObjectsAndProgram>::Iterator it = sorted_objects.begin() ;
			it != it_end ;
			it++)
		{
			// If the object has the same GPUProgram, add it to the "category"
			ObjectsAndProgram& obj_and_prog = (*it);
			if(program == obj_and_prog.program)
			{
				obj_and_prog.objects.pushBack(obj);
				found = true;
				break;
			}
		}

		// If we did not find a corresponding "category", we create a new one
		if(!found)
		{
			ObjectsAndProgram obj_and_prog;
			obj_and_prog.objects.pushBack(obj);
			obj_and_prog.program = program;
			sorted_objects.pushBack(obj_and_prog);
		}
	}

	// ----- Second step : copy the list back to the original objects array ------
	uint index = 0;
	List<ObjectsAndProgram>::Iterator it_end = sorted_objects.end();
	for(List<ObjectsAndProgram>::Iterator it = sorted_objects.begin() ;
		it != it_end ;
		it++)
	{
		ObjectsAndProgram& obj_and_prog = (*it);
		List<Object*>::Iterator it_obj_end = obj_and_prog.objects.end();
		for(List<Object*>::Iterator it_obj = obj_and_prog.objects.begin() ;
			it_obj != it_obj_end ;
			it_obj++)
		{
			Object* obj = (*it_obj);
			objects[index] = obj;
			index++;
		}
	}

	assert(index == nb_objects);

	// Separate opaque and transparent objects, as we may have broken
	// the separation:
	separateOpaqueTransparentObjects();
}
