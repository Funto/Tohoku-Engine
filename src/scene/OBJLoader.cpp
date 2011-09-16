// OBJLoader.cpp

#include "OBJLoader.h"
#include <fstream>
#include <string>
#include <iostream>
#include "../log/Log.h"
#include "../Common.h"
#include "Scene.h"
using namespace std;

OBJLoader::OBJLoader()
{
}

OBJLoader::~OBJLoader()
{
}

// Load a scene from a .obj file:
void OBJLoader::load(Scene* scene, const char* filename, ElementContainer::Type container_type)
{
	logError("not implemented !");
/*	scene->free();
	scene->setName(filename);

	// Open the file
	ifstream f(filename);
	if(!f.is_open())
	{
		logFailed("unable to load the requested file \"", filename, "\"");
		return;
	}

	// Create an element container, associate it with the scene
	// and start filling it:
	ElementContainer* elements = ElementContainer::create(container_type);
	elements->beginFilling();
	scene->setElements(elements);

	// For each line in the file:
	uint num_line=0;
	while(!f.eof())
	{
		num_line++;

		// Get a line and remove '\r' and/or '\n':
		string line;
		getline(f, line);
		line = line.substr(0, line.find_first_of("\n\r"));

		if(line.empty())
			continue;

		// Get the starting position (position of the first non-whitespace character)
		uint start_pos = line.find_first_not_of(" \t");
		if(start_pos == string::npos)
			continue;	// case of a blank line

		// Case of a commentary:
		if(line[start_pos] == '#')
			continue;

		// Get the first word of the line, which represents the type of command:
		string id_command = line.substr(start_pos, line.find_first_of(' '));

		// Object:
		if(id_command == "o")
		{
			// We do not do anything interesting of this line, as the name
			// of the scene is already given using the file name...
		}
		// Vertex:
		else if(id_command == "v")
		{
		}
		// Material:
		else if(id_command == "usemtl")
		{
		}
		// Face:
		else if(id_command == "f")
		{
		}
		// Texture coordinate:
		else if(id_command == "vt")
		{
		}
		else
			logWarn("command \"", id_command, "\" not handled at line ", num_line);
	}

	elements->endFilling();
*/
}
