// SceneLoader.h

#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include "ElementContainer.h"

class Scene;

class SceneLoader
{
public:
	SceneLoader();
	virtual ~SceneLoader();

	// Load a scene from a .dae or a .obj file:
	void load(Scene* scene, const char* filename, ElementContainer::Type container_type=ElementContainer::ARRAY);
};

#endif // SCENE_LOADER_H
