// Quad.h

#ifndef GL_QUAD_H
#define GL_QUAD_H

#include "../Common.h"
#include "glxw.h"

namespace glutil
{

// A fullscreen quad
class Quad
{
private:
	uint width;
	uint height;

	bool gpu_transfert_done;

	// VBO and VAO:
	GLuint id_vbo;
	GLuint id_vao;

	// Texture :
	GLuint id_texture;

	// Shaders and program :
	GLuint id_vertex;
	GLuint id_fragment;
	GLuint id_program;

	// Uniform :
	GLuint uniform_texunit;

	Pixel* pixels;

public:
	// Should be created AFTER OpenGL is initialized
	Quad(uint width, uint height);
	~Quad();

	uint getWidth() const {return width;}
	uint getHeight() const {return height;}

	void setPixels(Pixel* pixels);
	Pixel* getPixels() {return pixels;}

	// If we modify the pixels using the pointer we get through getPixels(),
	// we need to notify that the pixels have to be updated on the GPU.
	void markAsUpdated() {gpu_transfert_done = false;}
	void display();

private:
	void createTexture();
	void createProgram();
	void createVBOAndVAO();
};

}

#endif // GL_QUAD_H
