// PhotonVolumesRenderer.cpp

#include "PhotonVolumesRenderer.h"
#include "GLRaytracer.h"
#include "GLRaytracerConfig.h"
#include "GBuffer.h"
#include "../../utils/TGALoader.h"
#include <cassert>
#include <cmath>
using namespace std;

#define ATTRIB_POSITION 0

#define FRAG_DATA_COLOR 0

#define NB_VERTICES 12
#define NB_FACES 20

PhotonVolumesRenderer::PhotonVolumesRenderer(uint layers_width,
											 uint layers_height,
											 uint intersection_map_width,
											 uint intersection_map_height,
											 const string& kernel_filename,
											 const string& brdf_function)
: id_vao(0),
  id_vbo(0),
  id_index_buffer(0),
  id_kernel(0),
  kernel_filename(kernel_filename),
  brdf_function(brdf_function),
  program(NULL),
  layers_width(layers_width),
  layers_height(layers_height),
  intersection_map_width(intersection_map_width),
  intersection_map_height(intersection_map_height)
{
}

PhotonVolumesRenderer::~PhotonVolumesRenderer()
{
}

void PhotonVolumesRenderer::setup()
{
	loadKernelTexture();
	createVBOAndVAO();
	createProgram();
}

void PhotonVolumesRenderer::cleanup()
{
	assert(program != NULL);
	delete program;
	program = NULL;

	assert(id_kernel != 0);
	glDeleteTextures(1, &id_kernel);
	id_kernel = 0;

	assert(id_index_buffer != 0);
	glDeleteBuffers(1, &id_index_buffer);
	id_index_buffer = 0;

	assert(id_vbo != 0);
	glDeleteBuffers(1, &id_vbo);
	id_vbo = 0;

	assert(id_vao != 0);
	glDeleteVertexArrays(1, &id_vao);
	id_vao = 0;
}

void PhotonVolumesRenderer::run(const mat4& eye_view,
								const mat4& eye_proj,
								const GLRaytracer* gl_raytracer,
								const GBuffer* front_gbuffer,
								uint bounce_map_size)
{
	uint texunit = 0;

	// Enable depth test, backface culling and blending:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;
	glutil::Enable<GL_CULL_FACE>  cull_face_state;
	glutil::Enable<GL_BLEND>      blend_state;

	// Don't write to the Z-buffer:
	glDepthMask(GL_FALSE);

	// Additive blending:
	glBlendFunc(GL_ONE, GL_ONE);

	// Start using the program:
	program->use();

	// Send the uniforms:
	program->sendUniform("eye_view_matrix", eye_view);
	program->sendUniform("eye_proj_matrix", eye_proj);

	// Bind the textures to the corresponding texunits and send the corresponding uniforms:
#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getDebugTex(),      "tex_debug",    program, texunit);
#endif
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getPositionTex(),   "tex_position",   program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getPowerTex(),      "tex_power",      program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getComingDirTex(),  "tex_coming_dir", program, texunit);

	BIND_TEX(GL_TEXTURE_RECTANGLE, front_gbuffer->getTexPositions(), "tex_gbuffer_position",  program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, front_gbuffer->getTexNormals(),   "tex_gbuffer_normal",    program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, front_gbuffer->getTexDiffuse(),   "tex_gbuffer_diffuse",   program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, front_gbuffer->getTexSpecular(),  "tex_gbuffer_specular",  program, texunit);

	BIND_TEX(GL_TEXTURE_1D,        id_kernel,                        "tex_kernel",   program, texunit);

    program->validate();

	// Bind the VAO and draw
	glBindVertexArray(id_vao);
	glDrawElementsInstanced(GL_TRIANGLES, NB_FACES*3, GL_UNSIGNED_INT, 0, bounce_map_size*bounce_map_size);

	// Restore writing to the Z-buffer:
	glDepthMask(GL_TRUE);
}

void PhotonVolumesRenderer::createVBOAndVAO()
{
	// Reference about the icosahedron: http://en.wikipedia.org/wiki/Icosahedron

	const float phi = 0.5 * (1.0 + sqrt(5.0));	// golden ratio

	// Vertex coordinates for an icosahedron of edge length 2:
	GLfloat vertices[] = {
		// Vertices:
		// On the (x, y) plane (rectangle along the X axis):
		-phi, -1.0, 0.0,
		+phi, -1.0, 0.0,	//+phi, -1.0, 0.0,
		+phi, +1.0, 0.0,	//+phi, +1.0, 0.0,
		-phi, +1.0, 0.0,

		// On the (y, z) plane (rectangle along the Y axis):
		0.0, -phi, -1.0,
		0.0, +phi, -1.0,
		0.0, +phi, +1.0,
		0.0, -phi, +1.0,

		// On the (x, z) plane (rectangle along the Z axis):
		-1.0, 0.0, -phi,
		+1.0, 0.0, -phi,	//+1.0, 0.0, -phi,
		+1.0, 0.0, +phi,
		-1.0, 0.0, +phi,
	};

	const GLsizeiptr vertices_size = sizeof(vertices);
	assert(NB_VERTICES == (vertices_size / (3*sizeof(GLfloat))) );

	// Divide the vertices so that we get the coordinates for an icosahedron
	// of volume 1:
	const GLfloat wanted_length = pow(12.0 / (5.0*(3.0 + sqrt(5.0))), 1.0/3.0);	// length of an icosahedron
																				// of volume 1
	const GLfloat scale_factor = wanted_length / 2.0;
	for(uint i=0 ; i < NB_VERTICES*3 ; i++)
		vertices[i]*= scale_factor;

	GLuint faces[] = {
		1, 9, 2,	// Start
		1, 2, 10,
		3, 8, 0,
		3, 0, 11,

		4, 1, 7,
		4, 7, 0,
		5, 6, 2,
		5, 3, 6,

		11, 10, 6,
		11, 7, 10,
		8, 9, 4,
		8, 5, 9,

		7, 1, 10,
		7, 11, 0,
		11, 6, 3,
		10, 2, 6,

		1, 4, 9,
		2, 9, 5,
		0, 8, 4,
		3, 5, 8,
	};

	const GLsizeiptr faces_size = sizeof(faces);
	assert(NB_FACES == faces_size / (3*sizeof(GLuint)));

	// Create the VAO:
	glGenVertexArrays(1, &id_vao);

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Create the buffer for vertices and put the data into it:
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glBufferData(GL_ARRAY_BUFFER, vertices_size, (const GLvoid*)vertices, GL_STATIC_DRAW);

	// Create the buffer for indices and put the data into it:
	glGenBuffers(1, &id_index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id_index_buffer);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces_size, (const GLvoid*)faces, GL_STATIC_DRAW);

	// - enable attributes:
	glEnableVertexAttribArray(ATTRIB_POSITION);

	// - vertices pointer:
	glVertexAttribPointer(ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);
}

void PhotonVolumesRenderer::createProgram()
{
	stringstream ss;
	program = new glutil::GPUProgram("media/shaders/photon_volumes.vert",
									 "media/shaders/photon_volumes.frag");

#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	program->getPreprocessor()->addSymbol("_DEBUG_USE_DEBUG_FBO_ATTACHMENT_");
#endif
	program->getPreprocessor()->addSymbol(PreprocSym("_LAYERS_WIDTH_",  layers_width));
	program->getPreprocessor()->addSymbol(PreprocSym("_LAYERS_HEIGHT_", layers_height));

	program->getPreprocessor()->addSymbol(PreprocSym("_INTERSECTION_MAP_WIDTH_",  intersection_map_width));
	program->getPreprocessor()->addSymbol(PreprocSym("_INTERSECTION_MAP_HEIGHT_", intersection_map_height));

	program->getPreprocessor()->addSymbol(PreprocSym("_BRDF_FUNCTION_", brdf_function));

	bool ok = program->compileAndAttach();
	assert(ok);

	program->bindAttribLocations(ATTRIB_POSITION,  "vertex_position",
								 0,                NULL);
	program->bindFragDataLocation(FRAG_DATA_COLOR, "frag_color");

	ok &= program->link();
	assert(ok);

	// Set the uniform names:
	program->setUniformNames("eye_view_matrix",
							 "eye_proj_matrix",
							 "tex_debug",
							 "tex_position",
							 "tex_power",
							 "tex_coming_dir",
							 "tex_gbuffer_position",
							 "tex_gbuffer_normal",
							 "tex_gbuffer_diffuse",
							 "tex_gbuffer_specular",
							 "tex_kernel",
							 NULL);

	program->validate();
}

void PhotonVolumesRenderer::loadKernelTexture()
{
	// Load the TGA file:
	TGALoader tga;
	TGAErrorCode error = tga.loadFile(kernel_filename.c_str());
	assert(error == TGA_OK);
	assert(tga.getBpp() == 3);

	// Create the 1D texture:
	glGenTextures(1, &id_kernel);
	glBindTexture(GL_TEXTURE_1D, id_kernel);

	// We set the filter in case there is a driver requires it, but we don't use filtering anyway.
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage1D(
			GL_TEXTURE_1D,
			0,
			GL_RGB8,
			tga.getWidth(),
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			(const GLvoid*)(tga.getData()));
}
