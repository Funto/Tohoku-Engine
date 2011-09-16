// Geometry.cpp

#include "Geometry.h"
#include "../glutil/glutil.h"
#include <cstdlib>
#include <cstddef>
#include <cassert>
using namespace std;

// ---------------------------------------------------------------------
Geometry::Geometry()
: vertices(NULL),
  normals(NULL),
  texcoords(NULL),
  nb_vertices(0),
  id_vbo(0)
{
	for(uint i=0 ; i < NB_MAX_VAO ; i++)
		id_vaos[i] = 0;
}

Geometry::~Geometry()
{
	clear();
}

// Setter. We expect newly-allocated pointers and take ownership of those.
// BEWARE: Any VBO or VAO created before calling this method is deleted!
void Geometry::setVertices(uint nb_vertices, float* vertices, float* normals, float* texcoords)
{
	clear();

	this->nb_vertices = nb_vertices;
	this->vertices    = vertices;
	this->normals     = normals;
	this->texcoords   = texcoords;
}

// VBO management:
void Geometry::buildVBO()
{
	assert(vertices != NULL);

	// If the VBO is already created, just bind it and discard.
	if(id_vbo != 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, id_vbo);
		return;
	}

	// Get the size of a single vertex in bytes:
	GLint vertex_size = getVertexSize();
	GLint vertex_size_in_floats = vertex_size / sizeof(GLfloat);
	uint nb_floats = uint(nb_vertices * vertex_size_in_floats);

	// Generate an array containing the interleaved data for passing it to the
	// VBO (TODO: maybe we should use glMapBuffer() / glUnmapBuffer() for better performance...)
	GLfloat* interleaved_data = new GLfloat[nb_floats];

	// - vertices:
	uint start_offset = 0;

	{
		uint j=0;
		for(uint i=start_offset ; i < nb_floats ; i += vertex_size_in_floats)
		{
			interleaved_data[i+0] = vertices[j++];
			interleaved_data[i+1] = vertices[j++];
			interleaved_data[i+2] = vertices[j++];
		}

		start_offset += 3;
	}

	// - normals:
	if(normals != NULL)
	{
		uint j=0;
		for(uint i=start_offset ; i < nb_floats ; i += vertex_size_in_floats)
		{
			interleaved_data[i+0] = normals[j++];
			interleaved_data[i+1] = normals[j++];
			interleaved_data[i+2] = normals[j++];
		}

		start_offset += 3;
	}

	// - texcoords:
	if(texcoords != NULL)
	{
		uint j=0;
		for(uint i=start_offset ; i < nb_floats ; i += vertex_size_in_floats)
		{
			interleaved_data[i+0] = texcoords[j++];
			interleaved_data[i+1] = texcoords[j++];
		}

		start_offset += 2;
	}

	// Create a VBO
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	// Allocate space for the VBO and put data inside:
	GLsizeiptr total_size = sizeof(GLfloat) * nb_floats;
	glBufferData(GL_ARRAY_BUFFER, total_size, (const GLvoid*)interleaved_data, GL_STATIC_DRAW);

	// Cleanup:
	delete [] interleaved_data;
}

void Geometry::deleteVBO()
{
	// Check if there is no VAO created:
#ifndef NDEBUG
	for(uint i=0 ; i < NB_MAX_VAO ; i++)
	{
		assert(id_vaos[i] == 0);
	}
#endif

	if(id_vbo != 0)
	{
		glDeleteBuffers(1, &id_vbo);
		id_vbo = 0;
	}
}

// VAO management:
void Geometry::buildVAO(uint index,
						GLuint vertex_attrib,
						GLuint normal_attrib,
						GLuint texcoords_attrib)
{
	assert(index < NB_MAX_VAO);

	// If the VAO is already created, just bind it and discard.
	if(id_vaos[index] != 0)
	{
		glBindVertexArray(id_vaos[index]);
		return;
	}

	// Build the VAO:
	// - create the VAO
	glGenVertexArrays(1, &id_vaos[index]);
	glBindVertexArray(id_vaos[index]);

	// - build the VBO in case it is not already built, and let it bound:
	buildVBO();

	// - enable the indicated vertex attributes (recorded in the VAO state):
	glEnableVertexAttribArray(vertex_attrib);

	if(normal_attrib != 0)
		glEnableVertexAttribArray(normal_attrib);

	if(texcoords_attrib != 0)
		glEnableVertexAttribArray(texcoords_attrib);

	// Specify the strides/offsets in the VBO:
	GLsizei stride = GLsizei(getVertexSize());
	uint offset = 0;

	glVertexAttribPointer(   vertex_attrib,     3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*3;

	// - normals:
	if(normal_attrib != 0)
		glVertexAttribPointer(normal_attrib,    3, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*3;

	// - texcoords:
	if(texcoords_attrib != 0)
		glVertexAttribPointer(texcoords_attrib, 2, GL_FLOAT, GL_FALSE, stride, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*2;

	GL_CHECK();
}

void Geometry::deleteVAO(uint index)
{
	assert(index < NB_MAX_VAO);
	if(id_vaos[index] != 0)
	{
		glDeleteVertexArrays(1, &id_vaos[index]);
		id_vaos[index] = 0;
	}
}

// Clear everything: memory, VBO, VAOs...
void Geometry::clear()
{
	// Delete VAOs:
	for(uint i=0 ; i < NB_MAX_VAO ; i++)
		deleteVAO(i);

	// Delete the VBO:
	deleteVBO();

	// Clear the data:
	if(nb_vertices != 0)
	{
		delete [] vertices;
		delete [] normals;
		delete [] texcoords;
	}

	nb_vertices = 0;
	vertices = normals = texcoords = NULL;
}

// ---------------------------------------------------------------------
// Helper function for computing the strides for the VBO.
inline uint Geometry::getVertexSize() const
{
	assert(vertices != NULL);
	GLint stride = 0;

	stride += 3*sizeof(GLfloat);	// vertices

	if(normals != NULL)
		stride += 3*sizeof(GLfloat);	// normals

	if(texcoords != NULL)
		stride += 2*sizeof(GLfloat);	// texture coordinates

	return stride;
}
