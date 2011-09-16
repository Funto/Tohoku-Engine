// Renderer.cpp

#include "Renderer.h"
#include "../log/Log.h"
#include "../scene/Scene.h"
#include "../scene/ElementContainer.h"

Renderer::Renderer(uint width, uint height, const vec3& back_color)
: width(width), height(height), back_color(back_color)
{
}

Renderer::~Renderer()
{
}

// ---------------------------------------------------------------------
// Called when we change the scene
// Loading:
void Renderer::loadScene(Scene* scene)
{
	switch(scene->getElements()->getType())
	{
	case ElementContainer::ARRAY:
		loadSceneArray(scene);
		break;
	case ElementContainer::OCTREE:
		loadSceneOctree(scene);
		break;
	case ElementContainer::BVH:
		loadSceneBVH(scene);
		break;
	case ElementContainer::KD_TREE:
		loadSceneKdTree(scene);
		break;
	default:
		assert(false);
		break;
	}
}

void Renderer::loadSceneArray(Scene* scene)
{
	logInfo("load scene \"", scene->getName(), "\"");
}

void Renderer::loadSceneOctree(Scene* scene)
{
	logInfo("load scene \"", scene->getName(), "\"");
}

void Renderer::loadSceneBVH(Scene* scene)
{
	logInfo("load scene \"", scene->getName(), "\"");
}

void Renderer::loadSceneKdTree(Scene* scene)
{
	logInfo("load scene \"", scene->getName(), "\"");
}

// ---------------------------------------------------------------------
// Unloading:
void Renderer::unloadScene(Scene* scene)
{
	switch(scene->getElements()->getType())
	{
	case ElementContainer::ARRAY:
		unloadSceneArray(scene);
		break;
	case ElementContainer::OCTREE:
		unloadSceneOctree(scene);
		break;
	case ElementContainer::BVH:
		unloadSceneBVH(scene);
		break;
	case ElementContainer::KD_TREE:
		unloadSceneKdTree(scene);
		break;
	default:
		assert(false);
		break;
	}
}

void Renderer::unloadSceneArray(Scene* scene)
{
	logInfo("unload scene \"", scene->getName(), "\"");
}

void Renderer::unloadSceneOctree(Scene* scene)
{
	logInfo("unload scene \"", scene->getName(), "\"");
}

void Renderer::unloadSceneBVH(Scene* scene)
{
	logInfo("unload scene \"", scene->getName(), "\"");
}

void Renderer::unloadSceneKdTree(Scene* scene)
{
	logInfo("unload scene \"", scene->getName(), "\"");
}

// ---------------------------------------------------------------------
// Render a frame
// render() redirects to the correct render[Array|Octree|...]() function, and keeps track
// of the last rendered scene so as to call loadScene() / unloadScene() when necessary.
void Renderer::render(Scene* scene)
{
	assert(scene->getCamera() != NULL);
	assert(scene->getElements() != NULL);

	// Call render[Array|Octree|...]()
	switch(scene->getElements()->getType())
	{
	case ElementContainer::ARRAY:
		renderArray(scene);
		break;
	case ElementContainer::OCTREE:
		renderOctree(scene);
		break;
	case ElementContainer::BVH:
		renderBVH(scene);
		break;
	case ElementContainer::KD_TREE:
		renderKdTree(scene);
		break;
	default:
		assert(false);
		break;
	}
}

void Renderer::renderArray(Scene* scene)
{
}

void Renderer::renderOctree(Scene* scene)
{
}

void Renderer::renderBVH(Scene* scene)
{
}

void Renderer::renderKdTree(Scene* scene)
{
}

// Debug drawing for the renderers
void Renderer::debugDraw2D(Scene* scene)
{
}

void Renderer::debugDraw3D(Scene* scene)
{
}
