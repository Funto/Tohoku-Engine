// GBufferRenderer.h
// This class manages the rendering of a GBuffer to a full-screen quad.

#ifndef GBUFFER_RENDERER_H
#define GBUFFER_RENDERER_H

#include "../../glutil/GPUProgram.h"

class GBuffer;
class Camera;
class Light;

class GBufferRenderer
{
private:
	uint width;
	uint height;

	glutil::GPUProgram* program;

	GLuint id_vao;
	GLuint id_vbo;

public:
	GBufferRenderer(uint width,
					uint height);
	virtual ~GBufferRenderer();

	void load(const Preprocessor::SymbolList& preproc_syms);
	void unload();

	// Binds stuff, and lets the user know which one is the first texunit he can use.
	void bind(GBuffer* gbuffer,
			  Camera* camera,
			  Light** lights,
			  uint nb_lights,
			  bool bind_shadow_maps,
			  const vec3& back_color,
			  uint* first_valid_texunit=NULL);

	// Draw a full-screen quad while evaluating the given GBuffer.
	// IMPORTANT: bind() has to be called before render()!
	void render();

	// Getters:
	glutil::GPUProgram* getProgram() {return program;}
	const glutil::GPUProgram* getProgram() const {return program;}

private:
	void createProgram(const Preprocessor::SymbolList& additional_preproc_syms);
	void createVBOAndVAO();
};

#endif // GBUFFER_RENDERER_H
