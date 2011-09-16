// Scene.h

#ifndef SCENE_H
#define SCENE_H

#include <string>

class Camera;
class ElementContainer;

class Scene
{
private:
	Camera* camera;
	ElementContainer* elements;	// Elements : objects + lights
	std::string name;

public:
	Scene();
	virtual ~Scene();

	void free();

	void setCamera(Camera* camera);
	Camera* getCamera() const;

	void setElements(ElementContainer* elements);
	ElementContainer* getElements();
	const ElementContainer* getElements() const;

	void setName(const std::string& name);
	const std::string& getName() const;
};

#endif // SCENE_H
