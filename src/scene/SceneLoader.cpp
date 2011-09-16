// SceneLoader.cpp

#include "SceneLoader.h"
#include "DAELoader.h"
#include "OBJLoader.h"
#include "../Common.h"
#include "../log/Log.h"
#include <string>
using namespace std;

SceneLoader::SceneLoader()
{
}

SceneLoader::~SceneLoader()
{
}

// Load a scene from a .dae or a .obj file:
void SceneLoader::load(Scene* scene, const char* filename, ElementContainer::Type container_type)
{
	// Determine the file type:
	string str_filename = string(filename);
	string extension = str_filename.substr(str_filename.find_last_of('.'));

	uint len = extension.length();
	for(uint i=0 ; i < len ; i++)
		extension[i] = tolower(extension[i]);

	if(extension == ".dae")
	{
		DAELoader loader;
		loader.load(scene, filename, container_type);
	}
	else if(extension == ".obj")
	{
		OBJLoader loader;
		loader.load(scene, filename, container_type);
	}
	else
		logError("cannot load scene \"", filename, "\": unknown file type");
}
