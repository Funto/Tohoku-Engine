// DAELoader.cpp

#include "DAELoader.h"
#include "Camera.h"
#include "Geometry.h"
#include "Light.h"
#include "Material.h"
#include "MeshObject.h"
#include "Scene.h"
#include "Sphere.h"
#include "../log/Log.h"
#include "../tinyxml/tinyxml.h"
#include "../utils/StrManip.h"
#include "../Common.h"
#include <string>
#include <cassert>
#include "../glm/gtc/matrix_transform.hpp"
using namespace std;

DAELoader::DAELoader()
{
}

DAELoader::~DAELoader()
{
}

// ---------------------------------------------------------------------
void DAELoader::resetInternalState()
{
	nb_lights = 0;
	root = NULL;
	visual_scenes_library = NULL;
	lights_library = NULL;
	cameras_library = NULL;
	geometries_library = NULL;
	base_dir = "";
	elements = NULL;
}

// ---------------------------------------------------------------------
template <class T>
void readTransformation(TiXmlElement* node_element, T* obj)
{
	TiXmlHandle node_handle(node_element);

	if(	node_element->FirstChildElement("matrix") &&
		node_element->FirstChildElement("matrix")->GetText())
	{
		mat4 transform;
		getNumbersArray(node_handle.FirstChildElement("matrix").ToElement()->GetText(), &transform[0][0], NULL);
		transform = glm::transpose(transform);

		obj->setOrientation(mat3(transform));
		obj->setPosition(vec3(transform[3]));
	}
}

// ---------------------------------------------------------------------
void DAELoader::load(Scene* scene, const char* filename, ElementContainer::Type container_type)
{
	logInfo("loading scene \"", filename, "\"");

	TiXmlDocument doc;

	resetInternalState();

	scene->free();

	// Load the file and check if it is valid
	if(!doc.LoadFile(filename))
	{
		logFailed("unable to load the requested file \"", filename, "\"");
		return;
	}

	// Get the base directory of the file :
	base_dir = getBaseDirectory(filename);

	// Get the <COLLADA> mark and create an error if it is missing
	if(!(root = doc.FirstChildElement("COLLADA")))
	{
		logFailed("the file \"", filename, "\" does not have a <COLLADA> root node");
		return;
	}

	TiXmlHandle collada_handle = root;

	// Check if the "libraries" (<library_XXX>) are here and get the pointers to them.
	if(!collada_handle.FirstChildElement("library_visual_scenes").ToElement())
	{
		logFailed("the file \"", filename, "\" does not have a visual scene library");
		root = NULL;
		return;
	}
	else
		visual_scenes_library = collada_handle.FirstChild("library_visual_scenes").ToElement();
	if(collada_handle.FirstChild("library_geometries").ToElement())
		geometries_library = collada_handle.FirstChild("library_geometries").ToElement();

	if(collada_handle.FirstChild("library_cameras").ToElement())
		cameras_library = collada_handle.FirstChild("library_cameras").ToElement();

	if(collada_handle.FirstChild("library_lights").ToElement())
		lights_library = collada_handle.FirstChild("library_lights").ToElement();

	// Create the ElementContainer of the Scene and start filling it
	this->elements = ElementContainer::create(container_type);
	this->elements->beginFilling();
	scene->setElements(elements);

	// Load the first <visual_scene>, and emit a warning if there are more or less than 1 :
	TiXmlElement* visual_scene_element = visual_scenes_library->FirstChildElement("visual_scene");

	if(visual_scene_element == NULL)
	{
		logWarn("the file does not have any visual scene");
		return;
	}

	if(visual_scene_element->NextSiblingElement("visual_scene") != NULL)
		logWarn("the file has more than 1 visual scene : only the first one is used");

	scene->setName(filename);
	loadVisualScene(scene, visual_scene_element);

	// End of the filling of the elements of the scene :
	this->elements->endFilling();
}

// ---------------------------------------------------------------------

// Loads a <COLLADA>/<library_visual_scenes>/<visual_scene> (called by load())
// NB : we do not support hierarchical scene nodes, so that only the root nodes are read !
void DAELoader::loadVisualScene(Scene* scene, TiXmlElement* visual_scene_element)
{
	// For each node :
	for(TiXmlElement* node_element = visual_scene_element->FirstChildElement("node") ;
		node_element != NULL ;
		node_element = node_element->NextSiblingElement("node"))
	{
		TiXmlHandle node_handle = node_element;

		// Check the type of the node :
		// - case it's a camera :
		if(node_element->FirstChildElement("instance_camera"))
		{
			loadCamera(scene, node_element);
		}
		// - case it's a light :
		else if(node_element->FirstChildElement("instance_light"))
		{
			loadLight(scene, node_element);
		}
		// - case it's a mesh :
		else if(node_element->FirstChildElement("instance_geometry"))
		{
			loadMeshObject(scene, node_element);
		}
		// - case it's something else (sphere...)
		else if(node_element->FirstChildElement("extra"))
		{
			loadOther(scene, node_element);
		}
	}
}

// ---------------------------------------------------------------------
void DAELoader::loadCamera(Scene* scene, TiXmlElement* node_element)
{
	// If we haven't found a <library_cameras>, return
	if(!cameras_library)
		return;

	// XML structure : <library_cameras>/<camera>/<optics>/<technique_common>/<perspective>/[<yfov>, <znear>, <zfar>]

	// Get the instance's camera URL
	string url = safeString(node_element->FirstChildElement("instance_camera")->Attribute("url")).substr(1);

	// Run trough all the cameras
	bool found = false;

	for(TiXmlElement* camera_element = cameras_library->FirstChildElement("camera") ;
		camera_element != NULL && !found;
		camera_element = camera_element->NextSiblingElement("camera"))
	{
		// Check if the id matches the url
		string id = camera_element->Attribute("id");

		if(id != url)
			continue;
		else
			found = true;

		// We found the camera :

		// Get the <perspective> mark
		TiXmlElement* perspective_element = TiXmlHandle(camera_element)
											.FirstChildElement("optics")
											.FirstChildElement("technique_common")
											.FirstChildElement("perspective")
											.ToElement();
		if(!perspective_element)
			return;

		// Create the camera and assign it to the scene
		Camera* cam = new Camera();
		scene->setCamera(cam);

		// Get the position and orientation of the camera :
		readTransformation(node_element, cam);

		// Parse the camera's properties
		TiXmlElement* yfov_element = perspective_element->FirstChildElement("yfov");
		TiXmlElement* znear_element = perspective_element->FirstChildElement("znear");
		TiXmlElement* zfar_element = perspective_element->FirstChildElement("zfar");

		float fovy = 45.0;	// Except "aspect", all these values are default values, overriden
							// by the values in the COLLADA file
		float aspect = 640.0 / 480.0;	// TODO : CHANGE THIS
		float znear = 1.0;
		float zfar = 1000.0;

		if(yfov_element && yfov_element->GetText())
			strToNumber(yfov_element->GetText(), &fovy);

		if(znear_element && znear_element->GetText())
			strToNumber(znear_element->GetText(), &znear);

		if(zfar_element && zfar_element->GetText())
			strToNumber(zfar_element->GetText(), &zfar);

		// Update the camera's data
		cam->setProjection(fovy, aspect, znear, zfar);
	}
}

// ---------------------------------------------------------------------
void DAELoader::loadLight(Scene* scene, TiXmlElement* node_element)
{
	if(!lights_library)
		return;

	TiXmlHandle node_handle(node_element);

	// Get the instance's light URL
	string url = safeString(node_element->FirstChildElement("instance_light")->Attribute("url")).substr(1);

	// We run trought all the lights
	bool found = false;

	for(TiXmlElement* light_element = lights_library->FirstChildElement("light") ;
		light_element != NULL && !found;
		light_element = light_element->NextSiblingElement("light"))
	{
		// Check if the id matches the url
		string id = light_element->Attribute("id");

		if(id != url)
			continue;
		else
			found = true;

		// Create a light object :
		Light* light = new Light();
		light->setName(id);
		elements->addLight(light);

		// Get its transformation :
		readTransformation(node_element, light);

		// If we found a <node>/<extra>/<technique>/<param type="STRING" name="light">,
		// load the light's XML file.
		TiXmlElement* technique_element =  node_handle	.FirstChildElement("extra")
														.FirstChildElement("technique")
														.ToElement();

		if(technique_element != NULL)
		{
			for(TiXmlElement* param_element = technique_element->FirstChildElement("param") ;
				param_element != NULL ;
				param_element = param_element->NextSiblingElement("param"))
			{
				if(safeString(param_element->Attribute("type")) == "STRING")
				{
					if(safeString(param_element->Attribute("name")) == "light")
					{
						string light_name = safeString(param_element->GetText());

						light->loadFromXML(base_dir + string("/lights/") + light_name);
					}
				}
			}
		}
	}
}

// ---------------------------------------------------------------------
void DAELoader::loadMeshObject(Scene* scene, TiXmlElement* node_element)
{
	TiXmlHandle node_handle = node_element;

	// Create a mesh object :
	string name = safeString(node_element->Attribute("id"));
	MeshObject* obj = new MeshObject();
	obj->setName(name);

	// Get its transformation :
	readTransformation(node_element, obj);

	// We get the corresponding <geometry> node
	TiXmlElement* geometry_element = NULL;
	string geometry_url = node_element->FirstChildElement("instance_geometry")->Attribute("url");
	geometry_url = geometry_url.substr(1);

	for(geometry_element = geometries_library->FirstChildElement("geometry");
		geometry_element != NULL ;
		geometry_element = geometry_element->NextSiblingElement("geometry"))
	{
		if(safeString(geometry_element->Attribute("id")) == geometry_url)
			break;
	}

	// If we found the <geometry> node, we load it and create the corresponding scene node
	if(geometry_element == NULL)
		logWarn("<geometry id=\"", geometry_url, "\" not found");
	else
	{
		// We load the geometry
		Geometry* geo = loadGeometry(geometry_element);

		// We assign the geometry to the object
		obj->setGeometry(geo);
	}

	// Read the material
	readMaterial(node_element, obj);

	// Finally, add the object to the container.
	// NB: it's important to do this in the end, once the object is constructed,
	// as addObject() can rely on object's properties.
	elements->addObject(obj);
}

// ---------------------------------------------------------------------
void DAELoader::loadOther(Scene* scene, TiXmlElement* node_element)
{
	TiXmlHandle node_handle = node_element;

	TiXmlElement* extra_element = node_element->FirstChildElement("extra");
	assert(extra_element != NULL);

	TiXmlElement* technique_element = extra_element->FirstChildElement("technique");
	if(!technique_element)
		return;

	// Read the parameters :
	string type = "";
	float radius = 0.0;

	for(TiXmlElement* param_element = technique_element->FirstChildElement("param") ;
		param_element != NULL ;
		param_element = param_element->NextSiblingElement("param"))
	{
		string param_name = safeString(param_element->Attribute("name"));
		string param_type = safeString(param_element->Attribute("type"));

		if(param_type == "STRING")
		{
			if(param_name == "type")
				type = safeString(param_element->GetText());
		}
		else if(param_type == "FLOAT")
		{
			if(param_name == "radius")
				strToNumber(param_element->GetText(), &radius);
		}
	}

	// Create the object :
	string name = safeString(node_element->Attribute("id"));

	// - sphere :
	if(type == "sphere")
	{
		Sphere* sphere = new Sphere();
		sphere->setName(name);
		sphere->setRadius(radius);
		elements->addObject(sphere);

		// Get its transformation
		readTransformation(node_element, sphere);

		// Get its material
		readMaterial(node_element, sphere);
	}
}

// ---------------------------------------------------------------------
inline float* loadSource(TiXmlElement* source_element, uint* stride, int* count);

inline Geometry* createGeometry(	float* vertices_data,	uint nb_vertices,	uint vertices_stride,
									float* normals_data,		uint nb_normals,	uint normals_stride,
									float* texcoords_data,	uint nb_texcoords,	uint texcoords_stride,
									uint* triangles_indices,uint nb_triangles,	uint triangles_stride,
									uint vertex_offset,		uint normal_offset,	uint texcoord_offset);

// ---------------------------------------------------------------------
Geometry* DAELoader::loadGeometry(TiXmlElement* geometry_element)
{
	const TiXmlHandle& geometry_handle(geometry_element);

	uint* triangles_indices = NULL;
	float *vertices_data  = NULL, *normals_data  = NULL, *texcoords_data  = NULL;
	uint  nb_vertices     = 0,    nb_normals     = 0,    nb_texcoords     = 0,    nb_triangles     = 0;
	uint  vertices_stride = 0,    normals_stride = 0,    texcoords_stride = 0,    triangles_stride = 0;
	uint  vertex_offset   = 0,    normal_offset  = 0,    texcoords_offset = 0;

	TiXmlHandle mesh_handle = geometry_handle.FirstChildElement("mesh");

	// Read the number of triangles :
	mesh_handle.FirstChildElement("triangles").ToElement()->QueryValueAttribute("count", &nb_triangles);

	// Read the <input> marks :
	for(TiXmlElement* el = mesh_handle.FirstChildElement("triangles").FirstChildElement("input").ToElement() ;
		el != NULL ;
		el = el->NextSiblingElement("input"))
	{
		string semantic = safeString(el->Attribute("semantic"));
		string source = safeString(el->Attribute("source")).substr(1);

		// ----- VERTICES -----
		// Get the offset of the vertices and load the vertices :
		if(semantic == "VERTEX")
		{
			// We read the vertices offset
			el->QueryValueAttribute("offset", &vertex_offset);

			// We update the triangles stride
			triangles_stride++;

			// Loop through the <vertices> until we find the one with the right id :
			for(	TiXmlElement* vertices_el = mesh_handle.FirstChildElement("vertices").ToElement() ;
					vertices_el != NULL ;
					vertices_el = vertices_el->NextSiblingElement("vertices"))
			{
				if(source == vertices_el->Attribute("id"))
				{
					// Loop through the <input> in the <vertices> to find the one with the attribute
					// semantic="POSITION", and load the corresponding <source>
					for(	TiXmlElement* input_el = vertices_el->FirstChildElement("input") ;
							input_el != NULL ;
							input_el = input_el->NextSiblingElement("input"))
					{
						if(safeString(input_el->Attribute("semantic")) == "POSITION")
						{
							// Get the final name of the <source> to use
							string final_source = safeString(input_el->Attribute("source")).substr(1);

							// Find the <source> and load it
							for(	TiXmlElement* source_el = mesh_handle.FirstChildElement("source").ToElement() ;
									source_el != NULL ;
									source_el = source_el->NextSiblingElement("source"))
							{
								if(final_source == source_el->Attribute("id"))
								{
									int float_count = 0;
									vertices_data = loadSource(source_el, &vertices_stride, &float_count);
									nb_vertices = float_count / vertices_stride;
									break;
								}
							}
							break;
						}
					}
					break;
				}
			}
		}

		// ----- NORMALS -----
		// Get the offset of the normals and load the normals :
		else if(semantic == "NORMAL")
		{
			// We read the normals offset
			el->QueryValueAttribute("offset", &normal_offset);

			// We update the triangles stride
			triangles_stride++;

			// Look for the <source> and loading :
			for(	TiXmlElement* source_el = mesh_handle.FirstChildElement("source").ToElement() ;
					source_el != NULL ;
					source_el = source_el->NextSiblingElement("source"))
			{
				if(source == source_el->Attribute("id"))
				{
					int float_count = 0;
					normals_data = loadSource(source_el, &normals_stride, &float_count);
					nb_normals = float_count / normals_stride;
					break;
				}
			}
		}

		// ----- TEX COORDS -----
		// We get the texcoords offset and load them
		else if(semantic == "TEXCOORD")
		{
			// We read the texcoords offset
			el->QueryValueAttribute("offset", &texcoords_offset);

			// We update the triangles stride
			triangles_stride++;

			// Look for the <source> and loading :
			for(	TiXmlElement* source_el = mesh_handle.FirstChildElement("source").ToElement() ;
					source_el != NULL ;
					source_el = source_el->NextSiblingElement("source"))
			{
				if(source == source_el->Attribute("id"))
				{
					int float_count = 0;
					texcoords_data = loadSource(source_el, &texcoords_stride, &float_count);
					nb_texcoords = float_count / texcoords_stride;
					break;
				}
			}
		}
	}

	// Read the triangles indices : create and fill the array
	triangles_indices = new uint[nb_triangles * triangles_stride * 3];
	getNumbersArray(mesh_handle.FirstChildElement("triangles")
								.FirstChildElement("p")
								.FirstChild().ToText()->Value(),
					(unsigned int*)triangles_indices);

	// -------------------------------------

	// Create the Geometry :
	Geometry* geo = createGeometry(	vertices_data,		nb_vertices,	vertices_stride,
									normals_data,		nb_normals,		normals_stride,
									texcoords_data,		nb_texcoords,	texcoords_stride,
									triangles_indices,	nb_triangles,	triangles_stride,
									vertex_offset,		normal_offset,	texcoords_offset);

	// Free the memory
	if(nb_vertices != 0)
		delete [] vertices_data;

	if(nb_normals != 0)
		delete [] normals_data;

	if(nb_texcoords != 0)
		delete [] texcoords_data;

	if(nb_triangles != 0)
		delete [] triangles_indices;

	return geo;
}

// ---------------------------------------------------------------------
inline float* loadSource(TiXmlElement* source_element, uint* stride, int* count)
{
	float* array = NULL;
	int temp=0;

	// Get the number of floating point values
	source_element->FirstChildElement("float_array")->QueryIntAttribute("count", count);

	// Get the floating point values array
	array = new float[*count];
	getNumbersArray(source_element->FirstChildElement("float_array")->FirstChild()->ToText()->Value(), array);

	// Get the stride
	source_element->FirstChildElement("technique_common")
					->FirstChildElement("accessor")
					->ToElement()->QueryIntAttribute("stride", &temp);
	*stride = temp;

	return array;
}

// ---------------------------------------------------------------------
// This function creates the Geometry, based on the information coming from the COLLADA file.
// It "decompresses" the information.
inline Geometry* createGeometry(	float* vertices_data,	uint nb_vertices,	uint vertices_stride,
									float* normals_data,		uint nb_normals,	uint normals_stride,
									float* texcoords_data,	uint nb_texcoords,	uint texcoords_stride,
									uint* triangles_indices,uint nb_triangles,	uint triangles_stride,
									uint vertex_offset,		uint normal_offset,	uint texcoord_offset)
{
	Geometry* geo = new Geometry();

	assert(vertices_data != NULL);

	// Compute the number of vertices we will create
	// "triangles_stride" corresponds to the number of attributes per vertex
	int nb_created_vertices = nb_triangles * 3;

	// Create the arrays
	float* vertices = new float[nb_created_vertices*3];
	float* normals = (normals_data != NULL) ? new float[nb_created_vertices*3] : NULL;
	float* texcoords = (texcoords_data != NULL) ? new float[nb_created_vertices*2] : NULL;

	// For each triangle :
	for(uint i=0 ; i < nb_triangles ; i++)
	{
		uint num_vertex, num_copied_vertex;

		for(uint j=0 ; j < 3 ; j++)
		{
			// Vertex :
			num_vertex = i*3 + j;
			num_copied_vertex = triangles_indices[(i*3 + j)*triangles_stride + vertex_offset];
			vertices[num_vertex*3 + 0] = vertices_data[num_copied_vertex*3 + 0];	// x
			vertices[num_vertex*3 + 1] = vertices_data[num_copied_vertex*3 + 1];	// y
			vertices[num_vertex*3 + 2] = vertices_data[num_copied_vertex*3 + 2];	// z

			// Normal :
			if(normals != NULL)
			{
				uint num_copied_normal = triangles_indices[(i*3 + j)*triangles_stride + normal_offset];
				normals[num_vertex*3 + 0] = normals_data[num_copied_normal*3 + 0];	// x
				normals[num_vertex*3 + 1] = normals_data[num_copied_normal*3 + 1];	// y
				normals[num_vertex*3 + 2] = normals_data[num_copied_normal*3 + 2];	// z
			}

			// Texture coordinates :
			if(texcoords != NULL)
			{
				uint num_copied_texcoord = triangles_indices[(i*3 + j)*triangles_stride + texcoord_offset];
				texcoords[num_vertex*2 + 0] = texcoords_data[num_copied_texcoord*2 + 0];	// x
				texcoords[num_vertex*2 + 1] = texcoords_data[num_copied_texcoord*2 + 1];	// y
			}
		}
	}

	geo->setVertices(nb_created_vertices, vertices, normals, texcoords);

	return geo;
}

// ---------------------------------------------------------------------
void DAELoader::readMaterial(TiXmlElement* node_element, Object* obj)
{
	TiXmlHandle node_handle(node_element);

	// If we find a <node>/<extra>/<technique>/<param type="STRING" name="material">,
	// load the material.
	TiXmlElement* technique_element =  node_handle	.FirstChildElement("extra")
													.FirstChildElement("technique")
													.ToElement();

	if(technique_element == NULL)
	{
		logError("no <technique> (hence no material) found for node \"", node_element->Attribute("id"), "\"");
		return;
	}

	for(TiXmlElement* param_element = technique_element->FirstChildElement("param") ;
		param_element != NULL ;
		param_element = param_element->NextSiblingElement("param"))
	{
		if(safeString(param_element->Attribute("type")) == "STRING")
		{
			if(safeString(param_element->Attribute("name")) == "material")
			{
				string material_name = safeString(param_element->GetText());

				// Load the material, if there is one :
				Material* material = new Material();
				if(!material->loadFromXML(base_dir + string("/materials/") + material_name))
					delete material;
				else
					obj->setMaterial(material);
			}
		}
	}
}
