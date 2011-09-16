// ElementContainer.cpp

#include "ElementContainer.h"

#include "ArrayElementContainer.h"
#include "../log/Log.h"

// RTTI :
const char* ElementContainer::getTypeStr() const
{
	return getTypeStr(getType());
}

const char* ElementContainer::getTypeStr(ElementContainer::Type t)
{
	if     (t == ARRAY)   return "ARRAY";
	else if(t == OCTREE)  return "OCTREE";
	else if(t == BVH)     return "BVH";
	else if(t == KD_TREE) return "KD_TREE";
	else                  return "???";
}

// Factory :
ElementContainer* ElementContainer::create(Type type)
{
	switch(type)
	{
	case ARRAY:
		return new ArrayElementContainer();
	default:
		logWarn("element container type \"", getTypeStr(type), "\" not implemented, will probably crash !");
		return NULL;
	}
}
