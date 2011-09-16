// PhotonVolumesRenderer.h

#ifndef PHOTON_VOLUMES_RENDERER_H
#define PHOTON_VOLUMES_RENDERER_H

#include "../../glutil/GPUProgram.h"
#include <string>

class GLRaytracer;
class GBuffer;

class PhotonVolumesRenderer
{
private:
	GLuint id_vao;
	GLuint id_vbo;
	GLuint id_index_buffer;

	GLuint id_kernel;	// 1D texture representing the values of the kernel
	std::string kernel_filename;

	std::string brdf_function;

	glutil::GPUProgram* program;

	uint layers_width;
	uint layers_height;

	uint intersection_map_width;
	uint intersection_map_height;

public:
	PhotonVolumesRenderer(uint layers_width,
						  uint layers_height,
						  uint intersection_map_width,
						  uint intersection_map_height,
						  const std::string& kernel_filename,
						  const std::string& brdf_function);
	virtual ~PhotonVolumesRenderer();

	void setup();
	void cleanup();

	void run(const mat4& eye_view,
			 const mat4& eye_proj,
			 const GLRaytracer* gl_raytracer,
			 const GBuffer* front_gbuffer,
			 uint bounce_map_size);

private:
	void loadKernelTexture();
	void createVBOAndVAO();
	void createProgram();
};

#endif // PHOTON_VOLUMES_RENDERER_H
