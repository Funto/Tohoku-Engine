// DebugRenderer.cpp

#include "DebugRenderer.h"
#include "../CommonIndices.h"
#include "../ShaderLocations.h"
#include "../scene/Scene.h"
#include "../scene/ArrayElementContainer.h"
#include "../scene/Light.h"
#include "../scene/MeshObject.h"
#include "../scene/Material.h"
#include "../scene/Geometry.h"
#include "../scene/Camera.h"
#include "../scene/profiles/GeneralProfile.h"
#include "../utils/TGALoader.h"
#include <iostream>
using namespace std;

DebugRenderer::DebugRenderer(uint width, uint height, const vec3 &back_color, const string& brdf_function)
: Renderer(width, height, back_color),
  see_from_light(false),
  num_light_viewer(0),
#ifdef USE_DEBUG_TEXTURE
  id_debug(0),
#endif
  brdf_function(brdf_function)
{
}

DebugRenderer::~DebugRenderer()
{
}


// Called when we switch to this renderer
void DebugRenderer::setup()
{
#ifdef USE_DEBUG_TEXTURE
	TGALoader tga;
	if(tga.loadFile(DEBUG_TEXTURE_FILENAME) == TGA_OK)
	{
		assert(tga.getBpp() == 4);
		id_debug = glutil::createTextureRGBA8(tga.getWidth(), tga.getHeight(), (const GLubyte*)(tga.getData()));
	}
#endif
}

// Called when we switch to another renderer or close the program
void DebugRenderer::cleanup()
{
#ifdef USE_DEBUG_TEXTURE
	if(id_debug != 0)
		glDeleteTextures(1, &id_debug);
#endif
}

// Called when we change the scene
void DebugRenderer::loadSceneArray(Scene* scene)
{
	logInfo("setup scene \"", scene->getName(), "\"");

	// Get some pointers and values
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	uint nb_lights = elements->getNbLights();

	// Load the materials:
	// - create or complete the list(s) of additional preprocessor symbols:
	Preprocessor::SymbolList preproc_syms;

	// - add _NB_LIGHTS_:
	preproc_syms.push_back(PreprocSym("_NB_LIGHTS_", nb_lights));

	// - add _BRDF_FUNCTION_:
	preproc_syms.push_back(PreprocSym("_BRDF_FUNCTION_", brdf_function));

	// - add _FORWARD_SHADING_:
	preproc_syms.push_back("_FORWARD_SHADING_");

	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		// Get some pointers:
		Object* obj = objects[i];

		if(obj->getType() != Object::MESH)
			continue;

		MeshObject* mesh_obj = (MeshObject*)obj;
		Geometry* geo = mesh_obj->getGeometry();
		Material* mat = obj->getMaterial();
		GeneralProfile* profile = (GeneralProfile*)(mat->getProfile(GENERAL_PROFILE));

		// Load the material's general profile:
		profile->loadTextures();
		profile->loadProgram(preproc_syms);

		// Build the usual VAO (and VBO is done automatically if needed)
		GLuint vertex_attrib = GENERAL_PROFILE_ATTRIB_POSITION;
		GLuint normal_attrib = 0;
		GLuint texcoords_attrib = 0;

		if( ! profile->hasNormalMapping())
			normal_attrib = GENERAL_PROFILE_ATTRIB_NORMAL;

		if(profile->hasTextureMapping() || profile->hasNormalMapping())
			texcoords_attrib = GENERAL_PROFILE_ATTRIB_TEXCOORDS;

		geo->buildVAO(VAO_INDEX_RASTER, vertex_attrib, normal_attrib, texcoords_attrib);
	}

	// Sort objects by program
	elements->sortObjectsByProgram(GENERAL_PROFILE);
}

void DebugRenderer::unloadSceneArray(Scene* scene)
{
	logInfo("unload scene \"", scene->getName(), "\"");

	// Get some pointers and values
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects[i];

		if(obj->getType() != Object::MESH)
			continue;

		MeshObject* mesh_obj = (MeshObject*)obj;
		Geometry* geo = mesh_obj->getGeometry();

		// Unload the material
		GeneralProfile* profile = (GeneralProfile*)(mesh_obj->getMaterial()->getProfile(GENERAL_PROFILE));

		// Unload the material
		profile->unloadTextures();
		profile->unloadProgram();

		// Delete the VAO;
		geo->deleteVAO(VAO_INDEX_RASTER);
	}
}

// Render a frame
void DebugRenderer::renderArray(Scene* scene)
{
	// BEGIN DEBUG
	int width, height;
	glfwGetFramebufferSize(GLFWWindow::getInstance()->getWindow(), &width, &height);
	glViewport(0, 0, width, width);
	// END DEBUG

	// Enable depth testing and cullfacing:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;
	glutil::Enable<GL_CULL_FACE>  cull_face_state;

	const Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	vec3 back_color = getBackColor();

	// Clear the screen
	glClearColor(back_color.r, back_color.g, back_color.b, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutil::GPUProgram* prev_program = NULL;	// Used for tracking if the currently
												// bound program changes

	// ------------------ Render the scene -----------------
	// Get some pointers and values
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Compute the view and projection matrices:
	mat4 view_matrix;
	mat4 proj_matrix;

	if(see_from_light)
	{
		assert(nb_lights != 0);
		Light* l = lights[num_light_viewer];
		view_matrix = l->computeViewMatrix();
		proj_matrix = l->computeProjectionMatrix();
	}
	else
	{
		view_matrix = camera->computeViewMatrix();
		proj_matrix = camera->computeProjectionMatrix();
	}

	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects[i];

		if(obj->getType() != Object::MESH)
			continue;

		GeneralProfile* profile = (GeneralProfile*)(obj->getMaterial()->getProfile(GENERAL_PROFILE));
		glutil::GPUProgram* program = profile->getProgram();

		// If the current program changed, we start using
		// the new program
		if(prev_program != program)
		{
			program->use();

			// Bind the uniforms common for all programs:
			// - lights:
			for(uint i=0 ; i < nb_lights ; i++)
			{
				char uniform_name[50];

				// - position of the light:
				sprintf(uniform_name, "light_pos_%d", i);
				const vec3 light_pos = lights[i]->getPosition();
				program->sendUniform(uniform_name, light_pos, Hash::AT_RUNTIME);
			}

			// Update prev_program
			prev_program = program;
		}

		// Bind the uniforms and textures specific to the object:
		profile->bind();

		// ----------------------------------
		MeshObject* mesh_obj = (MeshObject*)obj;
		Geometry* geo = mesh_obj->getGeometry();

		// - compute the model matrix:
		mat4 model_matrix = mat4(mesh_obj->getOrientation());
		model_matrix[3] = vec4(mesh_obj->getPosition(), 1.0);

		// - send uniform variables:
		program->sendUniform("view_matrix",       view_matrix);
		program->sendUniform("model_matrix",      model_matrix);
		program->sendUniform("projection_matrix", proj_matrix);

		// - bind the VAO and draw
		glBindVertexArray(geo->getVAO(VAO_INDEX_RASTER));
		glDrawArrays(GL_TRIANGLES, 0, geo->getNbVertices());
	}

	// BEGIN DEBUG
	glViewport(0, 0, width, height);
	// END DEBUG
}

// Debug drawing for the renderers
void DebugRenderer::debugDraw2D(Scene* scene)
{
#ifdef USE_DEBUG_TEXTURE
	glutil::displayTexture2D(id_debug, 0, 0);
#endif
}

// Key event:'
void DebugRenderer::onKeyEvent(int key, int action)
{
	if(action != GLFW_RELEASE)
		return;

	if(key == 'L')
	{
		see_from_light = !see_from_light;
		cout << "DebugRenderer: see_from_light == " << (see_from_light ? "true" : "false") << endl;
	}
}
