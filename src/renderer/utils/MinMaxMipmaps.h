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

	glutil::GPUProgram* program_first;	// first layer
	glutil::GPUProgram* program_next;	// next layers

	GLuint id_vao;
	GLuint id_vbo;
	
	uint nb_layers;
	GLuint* id_fbo;
	GLuint* id_min_max_tex;	// TODO: use mipmaps

public:
	MinMaxMipmaps(uint target_width, uint target_height);
	virtual ~MinMaxMipmaps();

	void setup();
	void cleanup();

	void run(GLuint id_tex_position);
	
	uint getNbLayers() const {return nb_layers;}
	GLuint getMinMaxTex(int i) const {return id_min_max_tex[i];}
	
private:
	void createFBOs();
	void createPrograms();
	void createVBOAndVAO();
};

#endif // MIN_MAX_MIPMAPS_H
