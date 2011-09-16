// DAELoader.h

#ifndef DAE_LOADER_H
#define DAE_LOADER_H

#include <string>
#include "ElementContainer.h"

class Scene;
class Geometry;
class Material;
class TiXmlElement;
class ElementContainer;
class Object;

class DAELoader
{
public:
	DAELoader();
	virtual ~DAELoader();

	// Load a scene from a .dae file:
	void load(Scene* scene, const char* filename, ElementContainer::Type container_type=ElementContainer::ARRAY);

// ---------------------------------------------------------------------

private:
	void resetInternalState();

	// Internal functions called by load() :

	// Loads a <COLLADA>/<library_visual_scenes>/<visual_scene> (called by load())
	// NB : we do not support hierarchical scene nodes, so that only the root nodes are read !
	void loadVisualScene(Scene* scene, TiXmlElement* visual_scene_element);

	void loadCamera(Scene* scene, TiXmlElement* node_element);

	void loadLight(Scene* scene, TiXmlElement* node_element);

	void loadMeshObject(Scene* scene, TiXmlElement* node_element);

	void loadOther(Scene* scene, TiXmlElement* node_element);

	// This function creates the Geometry, based on the information coming from the COLLADA file.
	// It "decompresses" the information.
	Geometry* loadGeometry(TiXmlElement* geometry_element);

	void readMaterial(TiXmlElement* node_element, Object* obj);

	// Internal variables used only when we load
	int nb_lights;
	TiXmlElement* root;	// <COLLADA>
	TiXmlElement* visual_scenes_library;	// <library_visual_scenes>
	TiXmlElement* lights_library;	// <library_lights>
	TiXmlElement* cameras_library;	// <library_cameras>
	TiXmlElement* geometries_library;	// <library_geometries>
	std::string base_dir;	// directory of the .dae file
	ElementContainer* elements;	// elements of the scene : objects + lights
};

#endif // DAE_LOADER_H
