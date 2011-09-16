// Geometry.h
// Wraps the OpenGL VBO/VAO mechanism.
// There is only one VBO, shared between the VAOs.
// VBO management:
// - the VBO can be built and deleted by hand by calling buildVBO()/deleteVBO()
// - buildVBO() can be called several times, but only builds the VBO the first time
// - deleteVBO() effectively deletes the VBO, no matter the number of previous calls to buildVBO()
// - the VBO is automatically built when building a VAO, if it's not already built
// - the VBO is deleted when calling setVertices(), clear() or ~Geometry()
// VAO management:
// - the VAOs are built and deleted by hand by calling buildVAO()/deleteVAO()
// - buildVAO() can be called several times on the same slot, but only builds the VAO the first time
// - deleteVAO() effectively deletes the given VAO, no matter the number of previous calls to buildVAO()
// - they are deleted when calling setVertices(), clear() or ~Geometry()
// TODO: add tangent vectors.
// TODO: change the behavior so that to keep track of the number of calls to buildVAO()/deleteVAO()
// and buildVBO()/deleteVBO()

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "../glutil/glxw.h"
#include "../Common.h"
#include "../Boundaries.h"

class Geometry
{
private:
	float* vertices;	// x1, y1, z1, x2, y2, z2, ...etc

	float* normals;		// nx1, ny1, nz1, nx2, ny2, nz2, ...etc

	float* texcoords;	// u1, v1, u2, v2, ...etc

	uint nb_vertices;

	GLuint id_vaos[NB_MAX_VAO];

	GLuint id_vbo;	// We only have one VBO which contains all the data in an interleaved fashion, i.e:
					// x1,y1,z1, nx1,ny1,nz1, u1,v1,   x2,y2,z2, nx2,ny2,nz2, u2,v2, etc.
					// If, for example, texture coordinates are not specified, they are skipped from the VBO, i.e.:
					// x1,y1,z1, nx1,ny1,nz1,   x2,y2,z2, nx2,ny2,nz2, etc.

public:
	Geometry();
	virtual ~Geometry();

	// Setter. We expect newly-allocated pointers and take ownership of those.
	// BEWARE: Any VBO or VAO created before calling this method is deleted!
	void setVertices(uint nb_vertices, float* vertices, float* normals=NULL, float* texcoords=NULL);

	// Getters:
	const float* getVertices()  const {return vertices;}
	const float* getNormals()   const {return normals;}
	const float* getTexCoords() const {return texcoords;}

	uint getNbVertices() const {return nb_vertices;}

	// VBO management:
	void buildVBO();
	void deleteVBO();
	GLuint getVBO() const {return id_vbo;}

	// VAO management:
	void buildVAO(uint index,
				  GLuint vertex_attrib,
				  GLuint normal_attrib,
				  GLuint texcoords_attrib);
	void deleteVAO(uint index);
	GLuint getVAO(uint index) const {return id_vaos[index];}

	// Clear everything: memory, VBO, VAOs...
	void clear();

private:
	// Helper function for computing the strides for the VBO.
	// Returns the total size needed for specifying one vertex (which is the same
	// think as the stride for glVertexAttribPointer()).
	inline uint getVertexSize() const;
};

#endif // GEOMETRY_H
