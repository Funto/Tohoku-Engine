// TextureReducer.h

#ifndef TEXTUREREDUCER_H
#define TEXTUREREDUCER_H

#include "../../Common.h"
#include "../../glutil/GPUProgram.h"

class GLRaytracer;

class TextureReducer
{
private:
	uint target_width;
	uint target_height;

	glutil::GPUProgram* program;

	GLuint id_vao;
	GLuint id_vbo;

public:
	TextureReducer(uint target_width, uint target_height);
	virtual ~TextureReducer();

	void setup();
	void cleanup();

	void run(GLRaytracer* gl_raytracer, uint offset_x=0);
	void createProgram();
	void createVBOAndVAO();
};

#endif // TEXTUREREDUCER_H
