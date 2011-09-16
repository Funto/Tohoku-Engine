// Quad.cpp

#include "Quad.h"
#include <string.h>
#include <assert.h>
#include "glutil.h"

#define ATTRIB_POSITION 0
#define ATTRIB_TEXCOORDS 1

#define FRAG_DATA_COLOR 0

namespace glutil
{

Quad::Quad(uint width, uint height)
: width(width), height(height),
  gpu_transfert_done(false),
  id_vbo(0), id_vao(0)
{
	// Create an array of pixels
	this->pixels = new Pixel[width*height];

	// Setup the texture
	createTexture();

	// Setup the GPU program
	createProgram();

	// Setup the VBO and the VAO
	createVBOAndVAO();
}

Quad::~Quad()
{
	delete [] this->pixels;
	glDeleteTextures(1, &id_texture);

	glDeleteProgram(id_program);
	glDeleteShader(id_vertex);
	glDeleteShader(id_fragment);

	glDeleteBuffers(1, &id_vbo);
	glDeleteVertexArrays(1, &id_vao);
}

void Quad::setPixels(Pixel* pixels)
{
	memcpy(this->pixels, pixels, width*height*sizeof(Pixel));
	gpu_transfert_done = false;
}

void Quad::display()
{
	// Texture:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	if(!gpu_transfert_done)
	{
		glTexSubImage2D(GL_TEXTURE_RECTANGLE, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		GL_CHECK();
		gpu_transfert_done = true;
	}

	// Setup shader and its uniforms:
	glUseProgram(id_program);
	glUniform1i(uniform_texunit, 0);

	// Draw the VAO:
	const uint nb_vertices = 6;
	glBindVertexArray(id_vao);
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
}

void Quad::createTexture()
{
	// Check the texture's size
	GLint max_texture_size = 0;
	glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &max_texture_size);

	assert(width <= uint(max_texture_size));
	assert(height <= uint(max_texture_size));

	// Setup the texture
	glGenTextures(1, &id_texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	// Set the filter
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Create the texture
	glTexImage2D(
			GL_TEXTURE_RECTANGLE,
			0,
			GL_RGB,
			width, height,
			0,
			GL_RGB,
			GL_UNSIGNED_BYTE,
			pixels);
}

void Quad::createProgram()
{
	// Setup the GPU program :
	// - create shaders and specify sources
	id_vertex = glCreateShader(GL_VERTEX_SHADER);
	id_fragment = glCreateShader(GL_FRAGMENT_SHADER);

	glutil::loadShaderSource(id_vertex, "media/shaders/fsquad.vert");
	glutil::loadShaderSource(id_fragment, "media/shaders/fsquad.frag");

	// - compile shaders
	glCompileShader(id_vertex);
	printShaderLog(id_vertex, "vertex shader");

	glCompileShader(id_fragment);
	printShaderLog(id_vertex, "fragment shader");

	// - create program and attach shaders
	id_program = glCreateProgram();
	glAttachShader(id_program, id_vertex);
	glAttachShader(id_program, id_fragment);

	// - associate the vertex attributes names and the wanted locations
	glBindAttribLocation(id_program, ATTRIB_POSITION,  "vertex_position");
	glBindAttribLocation(id_program, ATTRIB_TEXCOORDS, "vertex_texcoords");

	// - associate the fragment shader's variables and the wanted locations
	glBindFragDataLocation(id_program, FRAG_DATA_COLOR, "frag_color");

	// - link the program
	glLinkProgram(id_program);
	printProgramLog(id_program);

	GL_CHECK();

	// - get the uniform(s) location(s)
	uniform_texunit = glGetUniformLocation(id_program, "texunit");

	// - check
	GL_CHECK();
	glutil::checkProgramValid(id_program);
}

void Quad::createVBOAndVAO()
{
	// Create the VAO:
	glGenVertexArrays(1, &id_vao);

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Data:
	GLfloat max_x = GLfloat(width);
	GLfloat max_y = GLfloat(height);

	const uint nb_vertices = 6;
	GLfloat buffer_data[] = {	// Vertex coordinates (x, y)
								-1.0f, -1.0f,	// triangle 1
								+1.0f, -1.0f,
								+1.0f, +1.0f,

								-1.0f, -1.0f,	// triangle 2
								+1.0f, +1.0f,
								-1.0f, +1.0f,

								// Texture coordinates (u, v)
								0.0f,  0.0f,	// triangle 1
								max_x, 0.0f,
								max_x, max_y,

								0.0f,  0.0f,	// triangle 2
								max_x, max_y,
								0.0f,  max_y};
	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	// - enable attributes:
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_TEXCOORDS);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;

	// - texcoords pointer:
	glVertexAttribPointer(ATTRIB_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;

	GL_CHECK();
}

}
