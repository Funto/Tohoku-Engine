// MinMaxMipmaps.h
  
#ifndef MIN_MAX_MIPMAPS_H
#define MIN_MAX_MIPMAPS_H

#include "../../Common.h"
#include "../../glutil/GPUProgram.h"

class MinMaxMipmaps
{
private:
	uint target_width;
	uint target_height;

	glutil::GPUProgram* program;

	GLuint id_vao;
	GLuint id_vbo;

public:
	MinMaxMipmaps(uint target_width, uint target_height);
	virtual ~MinMaxMipmaps();

	void setup();
	void cleanup();

	void run();
	void createProgram();
	void createVBOAndVAO();
};

#endif // MIN_MAX_MIPMAPS_H
