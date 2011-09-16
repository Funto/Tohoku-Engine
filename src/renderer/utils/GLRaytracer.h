// GLRaytracer.h

#ifndef GL_RAYTRACER_H
#define GL_RAYTRACER_H

#include "../../Debug.h"
#include "../../Common.h"
#include "../../glutil/glutil.h"
#include "TextureReducer.h"

class BounceMap;
class GBuffer;
class Camera;
class Light;
class TextureReducer;

class GLRaytracer
{
private:
	// Dimensions of the depth layers:
	uint layers_width;
	uint layers_height;

	// Dimensions of the target (FBO or screen for debugging)
	uint target_width;
	uint target_height;

	// Number of depth layers:
	uint nb_gbuffers;

	// FBO:
	GLuint id_fbo;
	GLuint id_debug_target;
	GLuint id_position;
	GLuint id_power;
	GLuint id_coming_dir;

	// VBO and VAO:
	GLuint id_vbo;
	GLuint id_vao;

	// Programs:
	glutil::GPUProgram* raytrace_bm_program;	// raytracing from the bounce map

	// Texture reducer:
	TextureReducer* texture_reducer;

	// Debug texture:
	GLuint id_debug;

public:
	GLRaytracer(uint layers_width,
				uint layers_height,
				uint nb_gbuffers);
	virtual ~GLRaytracer();

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
	GLuint getDebugTex()     const {return id_debug_target;}
	GLuint getPowerTex()     const {return id_power;}
	GLuint getPositionTex()  const {return id_position;}
	GLuint getComingDirTex() const {return id_coming_dir;}

private:
	void createFBO();
	void createVBOAndVAO();
	void createRaytraceBMProgram();
};

#endif // GL_RAYTRACER_H
