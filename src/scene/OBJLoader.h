// OBJLoader.h

#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H

#include "ElementContainer.h"

class Scene;

class OBJLoader
{
public:
	OBJLoader();
	virtual ~OBJLoader();

	// Load a scene from a .obj file:
	void load(Scene* scene, const char* filename, ElementContainer::Type container_type=ElementContainer::ARRAY);
};

#endif // OBJ_LOADER_H
