// ShadowMap.cpp

#include "ShadowMap.h"

#include "../../CommonIndices.h"
#include "../../ShaderLocations.h"
#include "../../scene/Light.h"
#include "../../scene/ArrayElementContainer.h"
#include "../../scene/MeshObject.h"
#include "../../scene/Geometry.h"
#include "../../scene/Scene.h"
#include "../../log/Log.h"
#include "../../glutil/glutil.h"
#include <cassert>
using namespace std;

// ---------------------------------------------------------------------
ShadowMap::ShadowMap(uint size)
: size(size),
  id_fbo(0),
  id_rb_color(0),
  id_depth(0),
  program(NULL)
{
	// Check the texture's size
	GLint max_texture_size = 0;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size);

	assert(size <= uint(max_texture_size));

	// Setup the color renderbuffer:
	id_rb_color = glutil::createRenderbufferRGBA8(size, size);

	// Setup the depth texture:
	id_depth = glutil::createTextureDepth(size, size);
	GL_CHECK();

	// Setup a FBO:
	glGenFramebuffers(1, &id_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, id_fbo);

	// Attach the renderbuffer and the texture:
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, id_rb_color);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D,   id_depth,    0);
	GL_CHECK();

	// We need to attach something to GL_COLOR_ATTACHMENT0 for the FBO to be complete,
	// but we never need to write to it, hence the glDrawBuffer(GL_NONE):
	glDrawBuffer(GL_NONE);

	// Check the FBO:
	GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
		logSuccess("FBO creation");
	else
		logError("FBO not complete");

	// Back to normal framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Setup the GPUProgram:
	setupGPUProgram();
}

// ---------------------------------------------------------------------
ShadowMap::~ShadowMap()
{
	glDeleteFramebuffers(1, &id_fbo);

	glDeleteRenderbuffers(1, &id_rb_color);
	glDeleteTextures(1, &id_depth);

	program = NULL;
}

// ---------------------------------------------------------------------

// We need to call these functions when loading / unloading a scene.
// They create/delete VAOs necessary for rendering the shadow map from
// the light and assign/remove shadow maps to all lights.
void ShadowMap::loadScene(Scene* scene, uint index_vao)
{
	ElementContainer* elements = scene->getElements();

	if(elements->getType() == ElementContainer::ARRAY)
		loadSceneArray((ArrayElementContainer*)elements, index_vao);
	else
		logWarn("container type \"", elements->getTypeStr(), "\" not supported for shadow mapping");
}

void ShadowMap::unloadScene(Scene* scene, uint index_vao)
{
	ElementContainer* elements = scene->getElements();

	if(elements->getType() == ElementContainer::ARRAY)
		unloadSceneArray((ArrayElementContainer*)elements, index_vao);
	else
		logWarn("container type \"", elements->getTypeStr(), "\" not supported for shadow mapping");
}

// ---------------------------------------------------------------------
void ShadowMap::loadSceneArray(ArrayElementContainer* elements, uint index_vao)
{
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Build the VBOs and VAOs:
	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		// Only select mesh objects:
		Object* obj = objects[i];
		if(obj->getType() != Object::MESH)
			continue;

		MeshObject* mesh_obj = (MeshObject*)obj;

		// Build the VAO (and the VBO is built automatically if needed):
		mesh_obj->getGeometry()->buildVAO(index_vao, SHADOW_MAP_ATTRIB_POSITION, 0, 0);
	}

	// Add shadow maps to each light:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		l->setUserData(LIGHT_DATA_SHADOW_MAP, new ShadowMap(l->getShadowMapSize()));
	}
}

void ShadowMap::unloadSceneArray(ArrayElementContainer* elements, uint index_vao)
{
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	// Delete the VAOs:
	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		// Only select mesh objects:
		Object* obj = objects[i];
		if(obj->getType() != Object::MESH)
			continue;

		MeshObject* mesh_obj = (MeshObject*)obj;

		// Delete the VAO:
		mesh_obj->getGeometry()->deleteVAO(index_vao);
	}

	// Remove the shadow maps for each light:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];
		l->setUserData(LIGHT_DATA_SHADOW_MAP, NULL);
	}
}

// ---------------------------------------------------------------------
// Rendering of all shadow maps:
void ShadowMap::renderShadowMaps(Scene* scene, uint vao_index)
{
	ElementContainer* elements = scene->getElements();

	if(elements->getType() == ElementContainer::ARRAY)
		renderShadowMapsArray((ArrayElementContainer*)elements, vao_index);
	else
		logWarn("container type \"", elements->getTypeStr(), "\" not supported for shadow mapping");
}

// Array version:
void ShadowMap::renderShadowMapsArray(ArrayElementContainer* elements, uint vao_index)
{
	// For each light, render the shadow map:
	Light** lights = elements->getLights();
	uint nb_lights = elements->getNbLights();

	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];

		assert(l->getUserData(LIGHT_DATA_SHADOW_MAP) != NULL);
		assert(l->getUserData(LIGHT_DATA_SHADOW_MAP)->getType() == LightData::SHADOW_MAP);

		ShadowMap* shadow_map = (ShadowMap*)(l->getUserData(LIGHT_DATA_SHADOW_MAP));
		shadow_map->renderFromLightArray(l, elements, vao_index);
	}
}

// ---------------------------------------------------------------------
// Render a scene from the point of view of a light
void ShadowMap::renderFromLight(const Light* light, Scene* scene, uint index_vao)
{
	ElementContainer* elements = scene->getElements();

	if(elements->getType() == ElementContainer::ARRAY)
		renderFromLightArray(light, (ArrayElementContainer*)elements, index_vao);
	else
		logWarn("shadow map rendering for non-array element containers not implemented");
}

void ShadowMap::renderFromLightArray(const Light* light, ArrayElementContainer* elements, uint index_vao)
{
	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// Set the viewport:
	glutil::SetViewport viewport(0, 0, size, size);

	// Enable depth testing and cullfacing:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;
	glutil::Enable<GL_CULL_FACE>  cull_face_state;
	//glutil::Disable<GL_CULL_FACE>  cull_face_state;	// DEBUG!!

	// Cull the front faces instead of the back faces.
	// This way, during shadow comparison, front faces are not flickering and
	// back faces are not lit anyway, so the result of the comparison is not important there.
	glCullFace(GL_FRONT);

	// Polygon offset is rarely needed because of the cullfacing below, but it can solve
	// some z-fighting issues:
	glutil::Enable<GL_POLYGON_OFFSET_FILL> offset_fill_state;
	glPolygonOffset(1, 1);

	// Clear the depth buffer:
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Get some pointers and values
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	// Compute the view and projection matrices:
	mat4 view_matrix = light->computeViewMatrix();
	mat4 proj_matrix = light->computeProjectionMatrix();

	// Start using the program:
	program->use();

	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects[i];

		if(obj->getType() != Object::MESH)
			continue;

		MeshObject* mesh_obj = (MeshObject*)obj;
		Geometry* geo = mesh_obj->getGeometry();

		// - compute the model matrix:
		mat4 model_matrix = mat4(mesh_obj->getOrientation());
		model_matrix[3] = vec4(mesh_obj->getPosition(), 1.0);

		// - compute the modelview matrix:
		mat4 modelview_matrix = view_matrix * model_matrix;

		// - send uniform variables:
		program->sendUniform("modelview_matrix",  modelview_matrix);
		program->sendUniform("projection_matrix", proj_matrix);

		// - bind the VAO and draw
		glBindVertexArray(mesh_obj->getGeometry()->getVAO(index_vao));
		glDrawArrays(GL_TRIANGLES, 0, geo->getNbVertices());
	}

	// Reactivate writing to the color buffer(s)
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	// Reset usual cullfacing
	glCullFace(GL_BACK);
}


// ---------------------------------------------------------------------
void ShadowMap::setupGPUProgram()
{
	bool ok = true;
	bool is_new = false;
	program = getProgramManager().getProgram(
					GPUProgramID("media/shaders/render_to_shadow_map.vert",
								 "media/shaders/render_to_shadow_map.frag"),
					&is_new);

	// If it is a new program, we should set the attrib location(s),
	// the frag data location(s), link it and set the uniform names/get their locations.
	if(is_new)
	{
		// - attrib location(s):
		program->bindAttribLocation(SHADOW_MAP_ATTRIB_POSITION, "vertex_position");

		// - frag data location(s):
		program->bindFragDataLocation(SHADOW_MAP_FRAG_DATA_COLOR, "frag_color");

		// - link the program:
		ok &= program->link();
		assert(ok);

		// - set the uniforms.
		program->setUniformNames("modelview_matrix",
								 "projection_matrix",
								 NULL);

		// - validate the program:
#ifndef NDEBUG
		program->validate();
#endif
	}
}
