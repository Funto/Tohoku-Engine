// PhotonsAdvancer.h

#ifndef PHOTONS_ADVANCER_H
#define PHOTONS_ADVANCER_H

#include "../../Common.h"
#include "../../glutil/glutil.h"

class GBuffer;
class Camera;
class Light;

class PhotonsAdvancer
{
private:
	// Dimensions of the depth layers:
	uint layers_width;
	uint layers_height;

	// Dimensions of the target (same as the size of the bounce map)
	uint target_width;
	uint target_height;

	// Number of depth layers:
	uint nb_gbuffers;

	// FBO:
	GLuint id_fbo;
	GLuint id_position;
	
	// VBO and VAO:
	GLuint id_vbo;
	GLuint id_vao;

	// Programs:
	glutil::GPUProgram* advance_photons_program;	// advancing photons from the bounce map

public:
	PhotonsAdvancer(uint layers_width,
					uint layers_height,
					uint nb_gbuffers);
	virtual ~PhotonsAdvancer();

	void setup();
	void cleanup();

	void run(Light* light,
			 GBuffer** gbuffers,
			 const mat4& eye_proj,
			 const mat4& eye_view,
			 float znear,
			 float zfar);

	// Getters:
	uint getLayersWidth()  const {return layers_width;}
	uint getLayersHeight() const {return layers_height;}

	uint getTargetWidth()  const {return target_width;}
	uint getTargetHeight() const {return target_height;}

	GLuint getFBO()          const {return id_fbo;}
	GLuint getPositionTex()  const {return id_position;}

private:
	void createFBO();
	void createVBOAndVAO();
	void createAdvancePhotonsProgram();
};

#endif // PHOTONS_ADVANCER_H
