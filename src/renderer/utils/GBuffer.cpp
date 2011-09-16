// GBuffer.cpp

#include "GBuffer.h"

#include "../RasterRenderer.h"
#include "../../scene/Camera.h"
#include "../../scene/Light.h"
#include "../../scene/ArrayElementContainer.h"
#include "../../scene/MeshObject.h"
#include "../../scene/Geometry.h"
#include "../../scene/Material.h"
#include "../../log/Log.h"
#include "../../glutil/glutil.h"
#include <cassert>
using namespace std;

// ---------------------------------------------------------------------
GBuffer::GBuffer(uint width, uint height, bool use_visibility_maps)
: width(width), height(height),
  use_visibility_maps(use_visibility_maps),
  id_fbo(0),
  id_position(0),
  id_normal(0),
  id_diffuse(0),
  id_specular(0),
  id_depth(0),
  id_visibility(0)
{
	// Check the texture's size
#ifndef NDEBUG
	GLint max_texture_size = 0;
	glGetIntegerv(GL_MAX_RECTANGLE_TEXTURE_SIZE, &max_texture_size);

	assert(width <= uint(max_texture_size));
	assert(height <= uint(max_texture_size));
#endif

	// Setup the textures:
	id_position = glutil::createTextureRectRGBAF(width, height, true);
	id_normal   = glutil::createTextureRectRGBAF(width, height, true);
	id_diffuse  = glutil::createTextureRectRGBAF(width, height, true);
	id_specular = glutil::createTextureRectRGBAF(width, height, true);
	id_depth = glutil::createTextureRectDepth(width, height);

	if(use_visibility_maps)
		id_visibility = glutil::createTextureRectRGBA8(width, height);

	GL_CHECK();

	// Setup a FBO:
	glGenFramebuffers(1, &id_fbo);
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// - attach the textures:
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_position, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_RECTANGLE, id_normal,   0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_RECTANGLE, id_diffuse,  0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_RECTANGLE, id_specular, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_RECTANGLE, id_depth,    0);

	if(use_visibility_maps)
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_RECTANGLE, id_visibility, 0);

	GL_CHECK();

	// - specify the draw buffers :
	if(use_visibility_maps)
	{
		static const GLenum draw_buffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
			GL_COLOR_ATTACHMENT4,
		};
		glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);
	}
	else
	{
		static const GLenum draw_buffers[] = {
			GL_COLOR_ATTACHMENT0,
			GL_COLOR_ATTACHMENT1,
			GL_COLOR_ATTACHMENT2,
			GL_COLOR_ATTACHMENT3,
		};
		glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);
	}

	// Check the FBO:
	GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
		logSuccess("FBO creation");
	else
		logError("FBO not complete");
}

// ---------------------------------------------------------------------
GBuffer::~GBuffer()
{
	glDeleteFramebuffers(1, &id_fbo);

	glDeleteTextures(1, &id_position);
	glDeleteTextures(1, &id_normal);
	glDeleteTextures(1, &id_specular);
	glDeleteTextures(1, &id_diffuse);
	glDeleteTextures(1, &id_depth);

	if(use_visibility_maps)
		glDeleteTextures(1, &id_visibility);
}

// ---------------------------------------------------------------------
void GBuffer::render(Scene* scene,
					 RasterRenderer* raster_renderer,
					 TextureBinding* added_tex_bindings,
					 uint nb_added_tex_bindings)
{
	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// Enable the depth test:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;

	raster_renderer->renderArrayExt(scene, vec3(0.0), NULL, added_tex_bindings, nb_added_tex_bindings);
}

void GBuffer::renderFromLight(const Light* light,
							  Scene* scene,
							  RasterRenderer* raster_renderer,
							  TextureBinding* added_tex_bindings,
							  uint nb_added_tex_bindings)
{
	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// Enable the depth test:
	glutil::Enable<GL_DEPTH_TEST> depth_test_state;

	raster_renderer->renderArrayExt(scene, vec3(0.0), light, added_tex_bindings, nb_added_tex_bindings);
}
