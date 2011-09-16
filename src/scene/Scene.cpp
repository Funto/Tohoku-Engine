// Scene.cpp

#include "Scene.h"
#include "Camera.h"
#include "ElementContainer.h"
#include <cstdlib>
using namespace std;

Scene::Scene()
: camera(NULL), elements(NULL)
{
}

Scene::~Scene()
{
	free();
}

// Delete internal data
void Scene::free()
{
	delete camera;
	this->camera = NULL;

	delete elements;
	this->elements = NULL;
}

// Camera
void Scene::setCamera(Camera* camera)
{
	delete this->camera;
	this->camera = camera;
}

Camera* Scene::getCamera() const
{
	return camera;
}

// Elements : objects + lights
void Scene::setElements(ElementContainer* elements)
{
	delete this->elements;
	this->elements = elements;
}

ElementContainer* Scene::getElements()
{
	return elements;
}

// Scene name :
void Scene::setName(const string& name)
{
	this->name = name;
}

const string& Scene::getName() const
{
	return name;
}
