// TextureBinding.cpp

#include "TextureBinding.h"
#include "TexunitManager.h"

// This static function:
// - binds textures and sets their parameters
// - sets the uniforms in the given program for indicating the texture units
// - updates the TexunitManager
void TextureBinding::bind(	TextureBinding* tex_bindings,
							uint nb_tex_bindings,
							glutil::GPUProgram* program,
							TexunitManager* texunit_manager)
{
	uint texunits[NB_MAX_TEXTURE_BINDINGS];
	texunit_manager->getFreeTexunits(texunits, nb_tex_bindings);

	for(uint i=0 ; i < nb_tex_bindings ; i++)
	{
		uint texunit = texunits[i];
		const TextureBinding& tex_binding = tex_bindings[i];

		// - bind the texture to the texunit:
		glActiveTexture(GL_TEXTURE0 + texunit);
		glBindTexture(tex_binding.target, tex_binding.id_texture);

		// - set the parameters:
		if( ! tex_binding.parameters.empty())
		{
			const ParameterList& parameters = tex_binding.parameters;
			ParameterList::const_iterator it_end = parameters.end();
			for(ParameterList::const_iterator it = parameters.begin() ;
				it != it_end ;
				it++)
			{
				glTexParameteri(tex_binding.target, it->param_name, it->param);
			}
		}

		// - set the corresponding shader uniform variable:
		program->sendUniform(tex_binding.sampler_name.c_str(), GLint(texunit), Hash::AT_RUNTIME);
	}
}
