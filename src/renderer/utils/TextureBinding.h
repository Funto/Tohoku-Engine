// TextureBinding.h
// This structure represents a texture "to bind".
// We use this to tell a renderer to bind a given texture and associate
// it with a given sampler name in the shader.
// Choosing the right texture unit to use is the job of the renderer,
// hence the fact that we do not have a "texunit" field in this structure.

#ifndef TEXTURE_BINDING_H
#define TEXTURE_BINDING_H

#include "../../glutil/glxw.h"
#include "../../Boundaries.h"
#include "../../glutil/GPUProgram.h"
#include <string>
#include <list>
#include <algorithm>

class TexunitManager;

struct TextureBinding
{
	// Arguments to a glTexParameteri() call:
	struct Parameter
	{
		GLenum param_name;
		GLint param;

		Parameter(GLenum param_name, GLint param)
			: param_name(param_name), param(param)
		{
		}

		Parameter& operator=(const Parameter& ref)
		{
			this->param_name = ref.param_name;
			this->param = ref.param;
			return *this;
		}
	};

	typedef std::list<Parameter> ParameterList;

	std::string sampler_name;	// Name of the corresponding uniform variable
	GLuint id_texture;
	GLenum target;	// GL_TEXTURE_2D, GL_TEXTURE_RECTANGLE...
	ParameterList parameters;

	// ---------------------------------------------------------------------
	// This static function:
	// - binds textures and sets their parameters
	// - sets the uniforms in the given program for indicating the texture units
	// - updates the TexunitManager
	static void bind(TextureBinding* tex_bindings,
					 uint nb_tex_bindings,
					 glutil::GPUProgram* program,
					 TexunitManager* texunit_manager);


	// ---------------------------------------------------------------------
	TextureBinding(const char* sampler_name="", GLuint id_texture=0, GLenum target=GL_TEXTURE_2D)
		: sampler_name(sampler_name), id_texture(id_texture), target(target), parameters()
	{
	}

	TextureBinding(const TextureBinding& ref)
	{
		*this = ref;
	}

	TextureBinding& operator=(const TextureBinding& ref)
	{
		this->sampler_name = ref.sampler_name;
		this->id_texture   = ref.id_texture;
		this->target       = ref.target;
		std::copy(ref.parameters.begin(), ref.parameters.end(), this->parameters.begin());
		return *this;
	}
};

#endif // TEXTURE_BINDING_H
