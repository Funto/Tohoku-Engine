// RasterRenderer.cpp

#include "RasterRenderer.h"
#include "../ShaderLocations.h"
#include "../glutil/glutil.h"
#include "../scene/ArrayElementContainer.h"
#include "../scene/Camera.h"
#include "../scene/GPUProgramManager.h"
#include "../scene/Light.h"
#include "../scene/Material.h"
#include "../scene/MeshObject.h"
#include "../scene/Geometry.h"
#include "../scene/Scene.h"
#include "../scene/profiles/GeneralProfile.h"
#include "../log/Log.h"
#include "../utils/StdListManip.h"
#include "utils/ShadowMap.h"
#include "utils/TexunitManager.h"
#include "utils/TextureBinding.h"
#include <cassert>
#include <sstream>
#include <cstring>

#ifdef USE_DEBUG_TEXTURE
#include "../utils/TGALoader.h"
#endif
using namespace std;

RasterRenderer::RasterRenderer(uint width, uint height,
							   const vec3& back_color,
							   bool use_shadow_mapping,
							   bool use_visibility_maps,
							   const string& brdf_function,
							   uint vao_index,
							   uint vao_index_shadow,
							   uint profile_index)
: Renderer(width, height, back_color),
  use_shadow_mapping(use_shadow_mapping),
  use_visibility_maps(use_visibility_maps),
#ifdef USE_DEBUG_TEXTURE
  id_debug(0),
#endif
  brdf_function(brdf_function),
  vao_index(vao_index),
  vao_index_shadow(vao_index_shadow),
  profile_index(profile_index)
{
}

RasterRenderer::~RasterRenderer()
{
}

// Called when we switch to this renderer
void RasterRenderer::setup()
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
void RasterRenderer::cleanup()
{
#ifdef USE_DEBUG_TEXTURE
	if(id_debug != 0)
		glDeleteTextures(1, &id_debug);
#endif
}

// Called when we change the scene
// Setup the scene: sort objects by programs, load shaders and build the VAOs and VBOs.
void RasterRenderer::loadSceneArray(Scene* scene)
{
	Preprocessor::SymbolList additional_preproc_syms;
	additional_preproc_syms.push_back(PreprocSym("_FORWARD_SHADING_", ""));
	loadSceneArrayExt(scene, additional_preproc_syms);
}

void RasterRenderer::loadSceneArrayExt(Scene* scene, const Preprocessor::SymbolList& additional_preproc_syms)
{
	logInfo("setup scene \"", scene->getName(), "\"");

	glutil::printGPUMemoryInfo("before loading the scene");

	// Get some pointers and values
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	uint nb_lights = elements->getNbLights();

	// Load the materials:
	// - create and complete the list(s) of additional preprocessor symbols:
	Preprocessor::SymbolList preproc_syms;
	vectCat(&preproc_syms, &additional_preproc_syms);

	// - add _NB_LIGHTS_:
	preproc_syms.push_back(PreprocSym("_NB_LIGHTS_", nb_lights));

	// - add _SHADOW_MAPPING_ if we use shadow mapping:
	if(use_shadow_mapping)
		preproc_syms.push_back(PreprocSym("_SHADOW_MAPPING_", ""));

	// - add _VISIBILITY_MAPS_ if we use visibility_maps:
	if(use_visibility_maps)
		preproc_syms.push_back(PreprocSym("_VISIBILITY_MAPS_", ""));

	// - add _BRDF_FUNCTION_:
	if(brdf_function != "")
		preproc_syms.push_back(PreprocSym("_BRDF_FUNCTION_", brdf_function));

	// - add _DEBUG_TEXTURE_ if we use the debug texture:
#ifdef USE_DEBUG_TEXTURE
	preproc_syms.push_back(PreprocSym("_DEBUG_TEXTURE_", ""));
#endif

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
		GeneralProfile* profile = (GeneralProfile*)(mat->getProfile(profile_index));

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

		geo->buildVAO(vao_index, vertex_attrib, normal_attrib, texcoords_attrib);
	}

	// Build the VBOs, VAOs, etc used for shadow mapping
	if(use_shadow_mapping)
		ShadowMap::loadSceneArray(elements, vao_index_shadow);

	// Sort objects by program
	elements->sortObjectsByProgram(profile_index);

	glutil::printGPUMemoryInfo("after loading the scene");
}

// Unload the materials of the previous scene
void RasterRenderer::unloadSceneArray(Scene* scene)
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
		GeneralProfile* profile = (GeneralProfile*)(mesh_obj->getMaterial()->getProfile(profile_index));

		// Unload the material
		profile->unloadTextures();
		profile->unloadProgram();

		// Delete the VAO:
		geo->deleteVAO(vao_index);
	}

	// In case we used shadow mapping, also delete the special VBOs and VAOs we used for it, and other stuff
	if(use_shadow_mapping)
		ShadowMap::unloadSceneArray(elements, vao_index_shadow);
}

// ---------------------------------------------------------------------
// Rendering, in case the container is an array container:
void RasterRenderer::renderArray(Scene *scene)
{
	renderArrayExt(scene, getBackColor(), NULL);	// from the viewpoint of the camera
}

void RasterRenderer::renderArrayExt(Scene* scene,
									const vec3& back_color,
									const Light* light_viewpoint,
									TextureBinding* added_tex_bindings,
									uint nb_added_tex_bindings)
{
	stringstream ss;

	// Enable depth testing and cullfacing:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;
	glutil::Enable<GL_CULL_FACE>  cull_face_state;

	const Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());

	// Clear the screen
	glClearColor(back_color.r, back_color.g, back_color.b, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutil::GPUProgram* prev_program = NULL;	// Used for tracking if the currently
												// bound program changes

	// --------------- Render the shadow maps --------------
	if(use_shadow_mapping)
		ShadowMap::renderShadowMapsArray(elements, vao_index_shadow);

	// ------------------ Render the scene -----------------
	// Get some pointers and values
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Compute the view and projection matrices:
	mat4 view_matrix;
	mat4 proj_matrix;

	if(light_viewpoint != NULL)
	{
		view_matrix = light_viewpoint->computeViewMatrix();
		proj_matrix = light_viewpoint->computeProjectionMatrix();
	}
	else
	{
		view_matrix = camera->computeViewMatrix();
		proj_matrix = camera->computeProjectionMatrix();
	}

	// Set the viewport:
	uint viewport_width  = getWidth();
	uint viewport_height = getHeight();
	if(light_viewpoint != NULL)
		viewport_width = viewport_height = light_viewpoint->getBounceMapSize();

	glutil::SetViewport viewport(0, 0, viewport_width, viewport_height);

	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects[i];

		if(obj->getType() != Object::MESH)
			continue;

		GeneralProfile* profile = (GeneralProfile*)(obj->getMaterial()->getProfile(profile_index));
		glutil::GPUProgram* program = profile->getProgram();

		// If the current program changed, we start using
		// the new program
		if(prev_program != program)
		{
			program->use();

			// Bind the uniforms common for all programs:
			// - lights (when not rendering to a GBuffer):
			if(program->getPreprocessor()->hasOriginalSymbol("_FORWARD_SHADING_"))
			{
				for(uint i=0 ; i < nb_lights ; i++)
				{
					Light* l = lights[i];

					// - position of the light:
					ss.str("");
					ss << "light_pos_" << i << flush;
					program->sendUniform(ss.str().c_str(), l->getPosition(), Hash::AT_RUNTIME);

					// - color of the light:
					ss.str("");
					ss << "light_color_" << i << flush;
					program->sendUniform(ss.str().c_str(), l->getColor(), Hash::AT_RUNTIME);
				}
			}

			// Update prev_program
			prev_program = program;
		}

		// Bind the uniforms and textures specific to the object:
		profile->bind();

		// Setup a texunit manager for the current profile:
		TexunitManager texunit_manager;
		texunit_manager.setTexunitsUsageFromProfile(profile);

		// Bind the debug texture to a free texture unit:
#ifdef USE_DEBUG_TEXTURE
		uint debug_texunit = texunit_manager.getFreeTexunit();
		glActiveTexture(GL_TEXTURE0 + debug_texunit);
		glBindTexture(GL_TEXTURE_2D, id_debug);
		program->sendUniform("tex_debug", GLint(debug_texunit));
#endif

		// Bind the shadow map textures and set the uniforms for indicating the texture units
		if(use_shadow_mapping)
			bindShadowMaps(lights, nb_lights, program, &texunit_manager);

		// Bind the additional textures:
		if(nb_added_tex_bindings != 0)
			TextureBinding::bind(added_tex_bindings, nb_added_tex_bindings, program, &texunit_manager);

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
		glBindVertexArray(geo->getVAO(vao_index));
		glDrawArrays(GL_TRIANGLES, 0, geo->getNbVertices());
	}
}

// ---------------------------------------------------------------------
void RasterRenderer::debugDraw2D(Scene* scene)
{
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Draw the shadow maps:
	if(use_shadow_mapping)
	{
		for(uint i=0 ; i < nb_lights ; i++)
		{
			ShadowMap* shadow_map = (ShadowMap*)(lights[i]->getUserData(LIGHT_DATA_SHADOW_MAP));
			glutil::displayTexture2D(shadow_map->getTexDepth(), i, 0);
		}
	}
}

// ---------------------------------------------------------------------
void RasterRenderer::debugDraw3D(Scene* scene)
{
	if(scene->getElements()->getType() != ElementContainer::ARRAY)
		return;

	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());

	const mat4& proj_matrix = scene->getCamera()->computeProjectionMatrix();
	const mat4& view_matrix = scene->getCamera()->computeViewMatrix();

	// Draw the frustums of the lights:
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	{
		glutil::Enable<GL_DEPTH_TEST> depth_test_state;
		for(uint i=0 ; i < nb_lights ; i++)
		{
			Light* l = lights[i];
			l->debugDrawFrustum(proj_matrix, view_matrix, vec3(1.0, 0.0, 0.0));
		}
	}
}

// ---------------------------------------------------------------------
// This function:
// - binds the shadow map textures
// - sets the uniforms for indicating the texture units
// - updates texunits_usage[] (read/write)
// It is called from the rendering function.
void RasterRenderer::bindShadowMaps(Light** lights,
									uint nb_lights,
									glutil::GPUProgram* program,
									TexunitManager* texunit_manager) const
{
	stringstream ss;

	// Get the necessary number of free texunits for binding the shadow maps:
	uint texunits[NB_MAX_TEXTURE_BINDINGS];
	texunit_manager->getFreeTexunits(texunits, nb_lights);

	// Bias matrix, used for shadow mapping. Lets us switch from the range [-1, 1] to [0, 1]
	// (0.5*x + 0.5)
	mat4 bias_matrix = mat4(vec4(0.5, 0.0, 0.0, 0.0),
							vec4(0.0, 0.5, 0.0, 0.0),
							vec4(0.0, 0.0, 0.5, 0.0),
							vec4(0.5, 0.5, 0.5, 1.0));

	// Bind the shadow map textures to the corresponding texture units
	// and send the corresponding uniforms to the current program:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		ShadowMap* shadow_map = (ShadowMap*)(l->getUserData(LIGHT_DATA_SHADOW_MAP));

		// Bind the texture to the texture unit
		glActiveTexture(GL_TEXTURE0 + texunits[i]);
		glBindTexture(GL_TEXTURE_2D, shadow_map->getTexDepth());

		// Set the parameters for shadow comparison and disable any possible mip-mapping:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,       GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,       GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

		// Send the texture unit as an uniform
		ss.str("");
		ss << "shadow_map_" << i << flush;
		program->sendUniform(ss.str().c_str(), GLint(texunits[i]), Hash::AT_RUNTIME);

		// Compute the "shadow matrix", which lets us do:
		// world space      => light view space		[light view matrix]
		// light view space => light clip space		[light proj matrix]
		// light clip space => shadow map space		[bias matrix]
		// Hence : shadow_matrix = bias_matrix * light_proj_matrix * light_view_matrix
		mat4 shadow_matrix = bias_matrix * l->computeProjectionMatrix() * l->computeViewMatrix();

		// Send the shadow matrix as a uniform to the program:
		ss.str("");
		ss << "shadow_matrix_" << i << flush;
		program->sendUniform(ss.str().c_str(), shadow_matrix, false, Hash::AT_RUNTIME);
	}
}
