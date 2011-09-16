// RAII.h - Resource Aquisition Is Initialisation

#ifndef RAII_H
#define RAII_H

#include "glxw.h"

namespace glutil
{

// ---------------------------------------------------------------------
// Enable
template <GLenum flag> struct Enable
{
	GLboolean state;

	Enable()
	{
		state = glIsEnabled(flag);

		if(!state)
			glEnable(flag);
	}

	~Enable()
	{
		if(!state)
			glDisable(flag);
	}
};

// ---------------------------------------------------------------------
// Disable
template <GLenum flag> struct Disable
{
	GLboolean state;

	Disable()
	{
		state = glIsEnabled(flag);

		if(state)
			glDisable(flag);
	}

	~Disable()
	{
		if(state)
			glEnable(flag);
	}
};

// ---------------------------------------------------------------------
// BindFramebuffer
struct BindFramebuffer
{
	GLint previous_binding;

	BindFramebuffer(GLint binding)
	{
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previous_binding);
		glBindFramebuffer(GL_FRAMEBUFFER, binding);
	}

	~BindFramebuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, previous_binding);
	}
};

// ---------------------------------------------------------------------
// TexParameteri
template <GLenum target, GLenum param_name, GLint param>
struct TexParameter
{
	GLint previous_param;

	TexParameter()
	{
		glGetTexParameteriv(target, param_name, &previous_param);
		glTexParameteri(target, param_name, param);
	}

	~TexParameter()
	{
		glTexParameteri(target, param_name, previous_param);
	}
};

// ---------------------------------------------------------------------
// TexParameterRebind - variant of TexParameter which also rebinds the texture
// before resetting its parameter.

// Specialized template for doing a "static if":
template <GLenum>
struct get_integer_param
{
};

template <>
struct get_integer_param<GL_TEXTURE_2D>
{
	enum {value = GL_TEXTURE_BINDING_2D};
};

template <>
struct get_integer_param<GL_TEXTURE_RECTANGLE>
{
	enum {value = GL_TEXTURE_BINDING_RECTANGLE};
};

template <GLenum target, GLenum param_name, GLint param>
struct TexParameterRebind
{
	GLint previous_param;
	GLint previous_binding;

	TexParameterRebind()
	{
		glGetIntegerv(get_integer_param<target>::value, &previous_binding);
		glGetTexParameteriv(target, param_name, &previous_param);
		glTexParameteri(target, param_name, param);
	}

	~TexParameterRebind()
	{
		glBindTexture(target, previous_binding);
		glTexParameteri(target, param_name, previous_param);
	}
};

// ---------------------------------------------------------------------
// SetViewport
struct SetViewport
{
	GLint prev_viewport[4];

	SetViewport(GLint x, GLint y, GLsizei width, GLsizei height)
	{
		glGetIntegerv(GL_VIEWPORT, prev_viewport);
		glViewport(x, y, width, height);
	}

	~SetViewport()
	{
		glViewport(prev_viewport[0], prev_viewport[1], prev_viewport[2], prev_viewport[3]);
	}
};

}

#endif // RAII_H
