// PhotonsMap.cpp

#include "PhotonsMap.h"

#include "GBuffer.h"
#include "BounceMap.h"
#include "../RasterRenderer.h"
#include "../../ShaderLocations.h"
#include "../../scene/Scene.h"
#include "../../scene/Camera.h"
#include "../../scene/Light.h"
#include "../../scene/ArrayElementContainer.h"
#include "../../scene/MeshObject.h"
#include "../../scene/Geometry.h"
#include "../../scene/Material.h"
#include "../../log/Log.h"
#include "../../glutil/glutil.h"
#include <cassert>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ---------------------------------------------------------------------
#define PHOTONS_MAP_TEXUNIT_BOUNCE_MAP_0    0
#define PHOTONS_MAP_TEXUNIT_BOUNCE_MAP_1    1
#define PHOTONS_MAP_TEXUNIT_LIGHT_POSITIONS 2
#define PHOTONS_MAP_TEXUNIT_SCREEN_DEPTH    3

#define DEBUG_PHOTONS_MAP_TEXUNIT_PHOTONS_MAP_0    0
#define DEBUG_PHOTONS_MAP_TEXUNIT_PHOTONS_MAP_1    1

// ---------------------------------------------------------------------
PhotonsMap::PhotonsMap(uint size)
: size(size),
  id_vao(0),
  id_vbo(0),
  program(NULL),
  id_fbo(0),
  id_output0(0),
  id_output1(0),
  id_debug_vao(0),
  id_debug_vbo(0),
  debug_program(NULL)
{
	createFBO();
	createProgram();
	createVBOAndVAO();
	
	createDebugProgram();
	createDebugVBOAndVAO();
}

// ---------------------------------------------------------------------
PhotonsMap::~PhotonsMap()
{
	// Delete the VBO/VAO:
	glDeleteVertexArrays(1, &id_vao);
	glDeleteBuffers(1, &id_vbo);

	// Decrement the reference counting of the GPU program:
	program = NULL;

	// Delete the FBO and the textures:
	glDeleteFramebuffers(1, &id_fbo);

	glDeleteTextures(1, &id_output0);
	glDeleteTextures(1, &id_output1);
	
	// Debug
	glDeleteVertexArrays(1, &id_debug_vao);
	glDeleteBuffers(1, &id_debug_vbo);
	debug_program = NULL;
}

// ---------------------------------------------------------------------
void PhotonsMap::run(const BounceMap* bounce_map,
                     const GBuffer* light_gbuffer,
                     const GBuffer* screen_gbuffer,
                     const mat4& eye_proj,
                     const mat4& eye_view,
                     uint nb_iterations)
{
	// TODO
	(void)nb_iterations;	// unused for now
	
	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(id_fbo);
	
	// Disable the depth test:
	glutil::Disable<GL_DEPTH_TEST> depth_test_state;

	// Bind the GPUProgram:
	program->use();
	GL_CHECK();

	// Send the uniforms:
	program->sendUniform("tex_bounce_map_0",    PHOTONS_MAP_TEXUNIT_BOUNCE_MAP_0);
	program->sendUniform("tex_bounce_map_1",    PHOTONS_MAP_TEXUNIT_BOUNCE_MAP_1);
	program->sendUniform("tex_light_positions", PHOTONS_MAP_TEXUNIT_LIGHT_POSITIONS);
	program->sendUniform("tex_screen_depth",    PHOTONS_MAP_TEXUNIT_SCREEN_DEPTH);
	
	// Bind the textures:
	glActiveTexture(GL_TEXTURE0 + PHOTONS_MAP_TEXUNIT_BOUNCE_MAP_0);
	glBindTexture(GL_TEXTURE_RECTANGLE, bounce_map->getTexOutput0());
	
	glActiveTexture(GL_TEXTURE0 + PHOTONS_MAP_TEXUNIT_BOUNCE_MAP_1);
	glBindTexture(GL_TEXTURE_RECTANGLE, bounce_map->getTexOutput1());
	
	glActiveTexture(GL_TEXTURE0 + PHOTONS_MAP_TEXUNIT_LIGHT_POSITIONS);
	glBindTexture(GL_TEXTURE_RECTANGLE, light_gbuffer->getTexPositions());
	
	glActiveTexture(GL_TEXTURE0 + PHOTONS_MAP_TEXUNIT_SCREEN_DEPTH);
	glBindTexture(GL_TEXTURE_RECTANGLE, screen_gbuffer->getTexDepth());
	
	// Compute and send the matrix used for going from light space to eye space:
	mat4 light_view = getLight()->computeViewMatrix();
	mat4 inv_light_view = glm::inverse(light_view);
	mat4 light_to_eye_matrix = eye_view * inv_light_view;
	program->sendUniform("light_to_eye_matrix", light_to_eye_matrix, false);
	
	// Send the eye projection matrix:
	program->sendUniform("eye_proj_matrix", eye_proj, false);
	
	// Draw the VAO:
	const uint nb_vertices = 6;
	glBindVertexArray(id_vao);
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
}

void PhotonsMap::debugDraw3D(const mat4& eye_proj_matrix)
{
	// Disable the depth test:
	glutil::Disable<GL_DEPTH_TEST> depth_test_state;
	
	const Light* l = getLight();
	uint bounce_map_size = l->getBounceMapSize();

	// Bind the GPUProgram:
	debug_program->use();
	GL_CHECK();

	// Send the uniforms:
	debug_program->sendUniform("debug_color",    vec4(0.0, 1.0, 0.0, 1.0));
	
	debug_program->sendUniform("tex_photons_map_0", DEBUG_PHOTONS_MAP_TEXUNIT_PHOTONS_MAP_0);
	debug_program->sendUniform("tex_photons_map_1", DEBUG_PHOTONS_MAP_TEXUNIT_PHOTONS_MAP_1);
	
	debug_program->sendUniform("photons_map_size", (GLint)(bounce_map_size));
	
	debug_program->sendUniform("eye_proj_matrix", eye_proj_matrix);
	
	// Bind the textures:
	glActiveTexture(GL_TEXTURE0 + DEBUG_PHOTONS_MAP_TEXUNIT_PHOTONS_MAP_0);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_output0);
	
	glActiveTexture(GL_TEXTURE0 + DEBUG_PHOTONS_MAP_TEXUNIT_PHOTONS_MAP_1);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_output1);
	
	// Draw the VAO:
	const uint nb_vertices = 2;
	glBindVertexArray(id_debug_vao);
	glDrawArraysInstanced(GL_LINES, 0, nb_vertices, bounce_map_size*bounce_map_size);
}

// ---------------------------------------------------------------------
void PhotonsMap::createFBO()
{
	// Check the texture's size
#ifndef NDEBUG
	GLint max_texture_size = 0;
	glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &max_texture_size);

	assert(size <= uint(max_texture_size));
#endif

	// Setup the textures:
	id_output0 = glutil::createTextureRectRGBAF(size, size, true);
	id_output1 = glutil::createTextureRectRGBAF(size, size, true);

	GL_CHECK();

	// Setup a FBO:
	glGenFramebuffers(1, &id_fbo);
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// - attach the textures:
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_output0, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, id_output1, 0);

	GL_CHECK();

	// - specify the draw buffers :
	static const GLenum draw_buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
	};
	glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);

	// Check the FBO:
	GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
		logSuccess("FBO creation");
	else
		logError("FBO not complete");
}

void PhotonsMap::createProgram()
{
	bool ok = true;
	bool is_new = false;

	program = getProgramManager().getProgram(
			GPUProgramID("media/shaders/ray_marching.vert", "media/shaders/ray_marching.frag"),
			&is_new);

	// If it is a new program, we should set the attrib location(s),
	// the frag data location(s), link it and set the uniform names/get their locations.
	if(is_new)
	{
		// - attrib location(s):
		program->bindAttribLocation(PHOTONS_MAP_ATTRIB_POSITION, "vertex_position");

		// - frag data location(s):
		program->bindFragDataLocations(PHOTONS_MAP_FRAG_DATA_OUTPUT0, "frag_output0",
									   PHOTONS_MAP_FRAG_DATA_OUTPUT1, "frag_output1",
									   0, NULL);

		// - link the program:
		ok &= program->link();
		assert(ok);

		// - set the uniforms.
		program->setUniformNames("tex_bounce_map_0",
								 "tex_bounce_map_1",
		                         "tex_light_positions",
		                         "tex_screen_depth",
		                         "light_to_eye_matrix",
		                         "eye_proj_matrix",
								 NULL);

		// - validate the program:
#ifndef NDEBUG
		program->validate();
#endif
	}
}

// ---------------------------------------------------------------------
void PhotonsMap::createVBOAndVAO()
{
	// Create the VAO:
	glGenVertexArrays(1, &id_vao);

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Vertex coordinates:
	GLfloat buffer_data[] = {	// Vertex coordinates (x, y)
								-1.0f, -1.0f,	// triangle 1
								+1.0f, -1.0f,
								+1.0f, +1.0f,

								-1.0f, -1.0f,	// triangle 2
								+1.0f, +1.0f,
								-1.0f, +1.0f,
							};

	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	const uint nb_vertices = 6;

	// - enable attributes:
	glEnableVertexAttribArray(PHOTONS_MAP_ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(PHOTONS_MAP_ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
}

// ---------------------------------------------------------------------
void PhotonsMap::createDebugProgram()
{
	bool ok = true;
	bool is_new = false;

	debug_program = getProgramManager().getProgram(
			GPUProgramID("media/shaders/debug/debug_photons_map.vert", "media/shaders/debug/debug_photons_map.frag"),
			&is_new);

	// If it is a new program, we should set the attrib location(s),
	// the frag data location(s), link it and set the uniform names/get their locations.
	if(is_new)
	{
		// - attrib location(s):
		debug_program->bindAttribLocation(PHOTONS_MAP_ATTRIB_POSITION, "vertex_position");

		// - frag data location(s):
		debug_program->bindFragDataLocations(DEBUG_PHOTONS_MAP_FRAG_DATA_COLOR, "frag_color",
									   0, NULL);

		// - link the program:
		ok &= debug_program->link();
		assert(ok);

		// - set the uniforms.
		debug_program->setUniformNames("debug_color",
		                               "tex_photons_map_0",
		                               "tex_photons_map_1",
		                               "photons_map_size",
		                               "eye_proj_matrix",
		                               NULL);

		// - validate the program:
#ifndef NDEBUG
		debug_program->validate();
#endif
	}
}

void PhotonsMap::createDebugVBOAndVAO()
{
	// Create the VAO:
	glGenVertexArrays(1, &id_debug_vao);

	// Bind the VAO:
	glBindVertexArray(id_debug_vao);

	// Vertex coordinates:
	GLfloat buffer_data[] = {	// Vertex coordinates (x, y)
								0.0f, 0.0f,
								1.0f, 1.0f,
							};

	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_debug_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_debug_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	const uint nb_vertices = 2;

	// - enable attributes:
	glEnableVertexAttribArray(DEBUG_PHOTONS_MAP_ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(DEBUG_PHOTONS_MAP_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
/*
	// Create the VAO:
	glGenVertexArrays(1, &id_debug_vao);

	// Bind the VAO:
	glBindVertexArray(id_debug_vao);

	// Vertex coordinates:
	GLfloat buffer_data[] = {	// Vertex coordinates (x, y)
								-1.0f, -1.0f,	// triangle 1
								+1.0f, -1.0f,
								+1.0f, +1.0f,

								-1.0f, -1.0f,	// triangle 2
								+1.0f, +1.0f,
								-1.0f, +1.0f,
							};

	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_debug_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_debug_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	const uint nb_vertices = 6;

	// - enable attributes:
	glEnableVertexAttribArray(DEBUG_PHOTONS_MAP_ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(DEBUG_PHOTONS_MAP_ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
*/
}
