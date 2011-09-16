// BounceMap.cpp

#include "BounceMap.h"

#include "GBuffer.h"
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
#define BOUNCE_MAP_TEXUNIT_POSITIONS 0
#define BOUNCE_MAP_TEXUNIT_NORMALS   1
#define BOUNCE_MAP_TEXUNIT_DIFFUSE   2
#define BOUNCE_MAP_TEXUNIT_SPECULAR  3

// ---------------------------------------------------------------------
BounceMap::BounceMap(uint size)
: size(size),
  id_vao(0),
  id_vbo(0),
  program(NULL),
  id_fbo(0),
  id_output0(0),
  id_output1(0)
//  should_get_query_result(false)
{
	createFBO();
	createProgram();
	createVBOAndVAO();

//	glGenQueries(1, &id_query);
}

// ---------------------------------------------------------------------
BounceMap::~BounceMap()
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

	// Occlusion query:
//	glDeleteQueries(1, &id_query);
}

// ---------------------------------------------------------------------
void BounceMap::renderFromGBuffer(const GBuffer* gbuffer, int num_iteration)
{
	// Get the light:
	const Light* l = getLight();

	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// Clear the bounce map:
	glClearColor(0.0, 0.0, 0.0, 0.0);
	//glClearColor(1.0, 0.0, 1.0, 0.0);	// DEBUG
	glClear(GL_COLOR_BUFFER_BIT);

	// Disable the depth test:
	glutil::Disable<GL_DEPTH_TEST> depth_test_state;

	// Bind the GPUProgram:
	program->use();

	// Send the uniforms:
	program->sendUniform("tex_positions", BOUNCE_MAP_TEXUNIT_POSITIONS);
	program->sendUniform("tex_normals",   BOUNCE_MAP_TEXUNIT_NORMALS);
	program->sendUniform("tex_diffuse",   BOUNCE_MAP_TEXUNIT_DIFFUSE);
	program->sendUniform("tex_specular",  BOUNCE_MAP_TEXUNIT_SPECULAR);

	program->sendUniform("light_color",  l->getColor());
	program->sendUniform("light_pos_ws", l->getPosition());
	program->sendUniform("seed_offset",  float(num_iteration));

	// Bind the textures:
	glActiveTexture(GL_TEXTURE0 + BOUNCE_MAP_TEXUNIT_POSITIONS);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexPositions());

	glActiveTexture(GL_TEXTURE0 + BOUNCE_MAP_TEXUNIT_NORMALS);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexNormals());

	glActiveTexture(GL_TEXTURE0 + BOUNCE_MAP_TEXUNIT_DIFFUSE);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexDiffuse());

	glActiveTexture(GL_TEXTURE0 + BOUNCE_MAP_TEXUNIT_SPECULAR);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexSpecular());

	// Start the occlusion query:
//	glBeginQuery(GL_SAMPLES_PASSED, id_query);

	// Draw the VAO:
	const uint nb_vertices = 6;
	glBindVertexArray(id_vao);
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);

	// Stop the occlusion query:
//	glEndQuery(GL_SAMPLES_PASSED);
//	should_get_query_result = true;
}

// ---------------------------------------------------------------------
void BounceMap::createFBO()
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

void BounceMap::createProgram()
{
	bool ok = true;
	bool is_new = false;

	program = getProgramManager().getProgram(
			GPUProgramID("media/shaders/bounce_map.vert", "media/shaders/bounce_map.frag"),
			&is_new);

	// If it is a new program, we should set the attrib location(s),
	// the frag data location(s), link it and set the uniform names/get their locations.
	if(is_new)
	{
		// - attrib location(s):
		program->bindAttribLocation(BOUNCE_MAP_ATTRIB_POSITION, "vertex_position");

		// - frag data location(s):
		program->bindFragDataLocations(BOUNCE_MAP_FRAG_DATA_OUTPUT0, "frag_output0",
									   BOUNCE_MAP_FRAG_DATA_OUTPUT1, "frag_output1",
									   0, NULL);

		// - link the program:
		ok &= program->link();
		assert(ok);

		// - set the uniforms.
		program->setUniformNames("tex_positions",
								 "tex_normals",
								 "tex_diffuse",
								 "tex_specular",
								 "light_color",
								 "light_pos_ws",
								 "seed_offset",
								 NULL);

		// - validate the program:
#ifndef NDEBUG
		program->validate();
#endif
	}
}

// ---------------------------------------------------------------------
void BounceMap::createVBOAndVAO()
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
	glEnableVertexAttribArray(BOUNCE_MAP_ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(BOUNCE_MAP_ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
}

// ---------------------------------------------------------------------
//uint BounceMap::getNbSurvivingPhotons()
//{
//	if(should_get_query_result)
//	{
//		glGetQueryObjectiv(id_query, GL_QUERY_RESULT, (GLint*)(&nb_surviving_photons));
//		should_get_query_result = false;
//	}

//	return nb_surviving_photons;
//}
