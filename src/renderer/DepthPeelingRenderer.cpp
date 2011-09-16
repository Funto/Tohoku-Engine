// DepthPeelingRenderer.cpp

#include "DepthPeelingRenderer.h"

#include "RasterRenderer.h"
#include "../CommonIndices.h"
#include "../ShaderLocations.h"
#include "../glutil/glutil.h"
#include "../scene/Scene.h"
#include "../scene/MeshObject.h"
#include "../scene/Light.h"
#include "../scene/Camera.h"
#include "../scene/ArrayElementContainer.h"
#include "../scene/Geometry.h"
#include "../scene/profiles/DepthPeelingProfile.h"
#include "../log/Log.h"
using namespace std;

#define DEPTH_PEELING_TEXUNIT_PREV_LAYER 0

// ---------------------------------------------------------------------
DepthPeelingRenderer::DepthPeelingRenderer(uint width, uint height, const vec3& back_color, uint nb_layers)
: Renderer(width, height, back_color),
  id_depth_layers(NULL),
  id_normal_layers(NULL),
  nb_layers(nb_layers),
  id_fbo(0)
{
	id_depth_layers  = new GLuint[nb_layers];
	id_normal_layers = new GLuint[nb_layers];

	for(uint i=0 ; i < nb_layers ; i++)
		id_depth_layers[i] = id_normal_layers[i] = 0;
}

DepthPeelingRenderer::~DepthPeelingRenderer()
{
	delete [] id_depth_layers;
	delete [] id_normal_layers;
}

// ---------------------------------------------------------------------
// Called when we switch to this renderer
void DepthPeelingRenderer::setup()
{
	uint width  = getWidth();
	uint height = getHeight();

	// Check the maximum texture size
#ifndef NDEBUG
	GLint max_texture_size = 0;
	glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &max_texture_size);

	assert(width <= uint(max_texture_size));
	assert(height <= uint(max_texture_size));
#endif

	// Setup the textures:
	for(uint i=0 ; i < nb_layers ; i++)
	{
		this->id_depth_layers[i]  = glutil::createTextureRectDepth(width, height);

		// See renderArrayExt() for the explanation
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

		this->id_normal_layers[i] = glutil::createTextureRectRGBA8(width, height);
	}
	GL_CHECK();

	// Setup the FBO:
	{
		glGenFramebuffers(1, &id_fbo);
		glutil::BindFramebuffer fbo_binding(id_fbo);

		// - attach the textures:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_normal_layers[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_RECTANGLE, id_depth_layers[0],  0);

		GL_CHECK();

		// - specify the draw buffers:
		static const GLenum draw_buffers[] = {
			GL_COLOR_ATTACHMENT0
		};

		glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);

		// - check the FBO:
		GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
			logSuccess("FBO creation");
		else
			logError("FBO not complete");
	}
}

// Called when we switch to another renderer or close the program
void DepthPeelingRenderer::cleanup()
{
	// Free the FBO:
	glDeleteFramebuffers(1, &id_fbo);
	id_fbo = 0;

	// Free the textures:
	glDeleteTextures(nb_layers, id_depth_layers);
	glDeleteTextures(nb_layers, id_normal_layers);

	for(uint i=0 ; i < nb_layers ; i++)
		id_depth_layers[i] = id_normal_layers[i] = 0;
}

// ---------------------------------------------------------------------
// Called when we change the scene
// Setup the scene: sort objects by programs and load shaders
void DepthPeelingRenderer::loadSceneArray(Scene* scene)
{
	loadSceneArrayExt(scene, true);
}

void DepthPeelingRenderer::loadSceneArrayExt(Scene* scene, bool sort_objects)
{
	logInfo("setup scene \"", scene->getName(), "\"");

	// Get some pointers and values
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

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
		DepthPeelingProfile* profile = (DepthPeelingProfile*)(mat->getProfile(DEPTH_PEELING_PROFILE));

		// Assert the texunits specific to this renderer are not marked as used in the material:
#ifndef NDEBUG
		{
			const uint texunits[] = {DEPTH_PEELING_TEXUNIT_PREV_LAYER};
			const uint nb_texunits = sizeof(texunits) / sizeof(uint);
			profile->assertTexunitsUnused(texunits, nb_texunits);
		}
#endif

		// Load the material's depth peeling profile:
		profile->loadTextures();
		profile->loadProgram();	// no additional preprocessor symbols needed

		// Build the usual VAO (and VBO is done automatically if needed)
		GLuint vertex_attrib = DEPTH_PEELING_ATTRIB_POSITION;
		GLuint normal_attrib = 0;
		GLuint texcoords_attrib = 0;

		if(profile->hasNormalMapping())
			texcoords_attrib = DEPTH_PEELING_ATTRIB_TEXCOORDS;
		else
			normal_attrib = DEPTH_PEELING_ATTRIB_NORMAL;

		geo->buildVAO(VAO_INDEX_DEPTH_PEELING, vertex_attrib, normal_attrib, texcoords_attrib);
	}

	// Sort objects by program
	if(sort_objects)
		elements->sortObjectsByProgram(DEPTH_PEELING_PROFILE);
}

// ---------------------------------------------------------------------
// Unload the materials of the previous scene
void DepthPeelingRenderer::unloadSceneArray(Scene* scene)
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
		DepthPeelingProfile* profile = (DepthPeelingProfile*)(mesh_obj->getMaterial()->getProfile(DEPTH_PEELING_PROFILE));

		// Unload the material
		profile->unloadTextures();
		profile->unloadProgram();

		// Delete the VAO:
		geo->deleteVAO(VAO_INDEX_DEPTH_PEELING);
	}
}

// ---------------------------------------------------------------------
// Render one frame
void DepthPeelingRenderer::renderArrayExt(Scene* scene, GLuint id_front_depth)
{
	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// Enable depth testing and cullfacing:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;
	glutil::Enable<GL_CULL_FACE>  cull_face_state;

	// Get some pointers/values:
	const Camera* camera = scene->getCamera();
	ArrayElementContainer* elements = (ArrayElementContainer*)(scene->getElements());
	Object** objects = elements->getObjects();
	uint nb_objects = elements->getNbObjects();

	// Compute the view and projection matrices :
	mat4 view_matrix = camera->computeViewMatrix();
	mat4 proj_matrix = camera->computeProjectionMatrix();

	// ---------- First pass ----------
	// If no front depth layer is specified (debug):
	if(id_front_depth == 0)
	{
		// Clear the first layer for using it as the base for comparisons in the subsequent passes
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_normal_layers[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_RECTANGLE, id_depth_layers[0],  0);

		glClearDepth(0.0);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearDepth(1.0);
	}
	// If id_front_depth is specified, use it for the first pass (actually, it's the second,
	// as the first has been done by the user of this renderer...)
	else
	{
		glCullFace(GL_FRONT);	// cull the front faces, as this is the second pass

		// Bind the specified front depth layer for reading:
		glActiveTexture(GL_TEXTURE0 + DEPTH_PEELING_TEXUNIT_PREV_LAYER);
		glBindTexture(GL_TEXTURE_RECTANGLE, id_front_depth);

		// For depth peeling, we keep the fragment if and only if:
		//             D_ref > D_t (GL_GREATER)
		// <=> current depth > previous depth
		// No need for any offset because of the alternating frontface/backface culling.
		glutil::TexParameterRebind<GL_TEXTURE_RECTANGLE, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE> tex_param1;
		glutil::TexParameterRebind<GL_TEXTURE_RECTANGLE, GL_TEXTURE_COMPARE_FUNC, GL_GREATER>                tex_param2;

		// Bind the first layer for writing
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_normal_layers[0], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_RECTANGLE, id_depth_layers[0],  0);

		// Draw the geometry:
		singlePassArray(objects, nb_objects, view_matrix, proj_matrix);
	}

	// ---------- Second to last passes ----------
	for(uint num_layer = 1; num_layer < nb_layers ; num_layer++)
	{
		// We alternatively cull the back or the front face:
		glCullFace((num_layer % 2 == 0) ? GL_FRONT : GL_BACK);

		// Bind the previous layer's depth buffer for reading:
		glActiveTexture(GL_TEXTURE0 + DEPTH_PEELING_TEXUNIT_PREV_LAYER);
		glBindTexture(GL_TEXTURE_RECTANGLE, id_depth_layers[num_layer-1]);

		// Bind the next layer for writing:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_normal_layers[num_layer], 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_RECTANGLE, id_depth_layers[num_layer],  0);

		// Draw the geometry:
		singlePassArray(objects, nb_objects, view_matrix, proj_matrix);
	}

	// Restore the normal backface culling:
	glCullFace(GL_BACK);
}

void DepthPeelingRenderer::renderArray(Scene* scene)
{
	renderArrayExt(scene, 0);
}

// ---------------------------------------------------------------------
// Debug drawing
void DepthPeelingRenderer::debugDraw2D(Scene* scene)
{
	debugDraw2DExt(scene, 0);
}

void DepthPeelingRenderer::debugDraw2DExt(Scene* scene, GLuint id_front_depth, GLuint id_front_normal)
{
	uint w = getWidth();
	uint h = getHeight();

	uint x = 0;
	uint y = 0;
	uint offset = 0;

	if(id_front_depth != 0)
	{
		x = offset % DEBUG_RECT_FACTOR;
		y = offset / DEBUG_RECT_FACTOR;
		glutil::displayTextureRect(id_front_depth, x, y, w, h);
		offset++;
	}

	if(id_front_normal != 0)
	{
		x = offset % DEBUG_RECT_FACTOR;
		y = offset / DEBUG_RECT_FACTOR;
		glutil::displayTextureRect(id_front_normal, x, y, w, h);
		offset++;
	}

	for(uint i=0 ; i < nb_layers ; i++)
	{
		x = (i*2 + 0 + offset) % DEBUG_RECT_FACTOR;
		y = (i*2 + 0 + offset) / DEBUG_RECT_FACTOR;
		glutil::displayTextureRect(id_depth_layers[i], x, y, w, h);

		x = (i*2 + 1 + offset) % DEBUG_RECT_FACTOR;
		y = (i*2 + 1 + offset) / DEBUG_RECT_FACTOR;
		glutil::displayTextureRect(id_normal_layers[i], x, y, w, h);
	}
}

// ---------------------------------------------------------------------
void DepthPeelingRenderer::singlePassArray(Object** objects,
										   uint nb_objects,
										   const mat4& view_matrix,
										   const mat4& proj_matrix)
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glutil::GPUProgram* prev_program = NULL;	// Used for tracking if the currently
												// bound program changes

	// For each mesh object:
	for(uint i=0 ; i < nb_objects ; i++)
	{
		Object* obj = objects[i];

		if(obj->getType() != Object::MESH)
			continue;

		DepthPeelingProfile* profile = (DepthPeelingProfile*)(obj->getMaterial()->getProfile(DEPTH_PEELING_PROFILE));
		glutil::GPUProgram* program = profile->getProgram();

		// If the current program changed, we start using
		// the new program
		if(prev_program != program)
		{
			program->use();

			// Bind the uniforms common for all programs:
			// - previous depth layer:
			program->sendUniform("tex_prev_depth_layer", DEPTH_PEELING_TEXUNIT_PREV_LAYER);

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
		glBindVertexArray(geo->getVAO(VAO_INDEX_DEPTH_PEELING));
		glDrawArrays(GL_TRIANGLES, 0, geo->getNbVertices());
	}
}
