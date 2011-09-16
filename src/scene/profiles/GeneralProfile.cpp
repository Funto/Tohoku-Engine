// GeneralProfile.cpp

#include "GeneralProfile.h"
#include "../../Boundaries.h"
#include "../../ShaderLocations.h"
#include "../../utils/StdListManip.h"
using namespace std;

GeneralProfile::GeneralProfile()
{
}

GeneralProfile::~GeneralProfile()
{
}

void GeneralProfile::onNewProgram(glutil::GPUProgram* program)
{
	// On creation of a new program:
	// - set the attrib locations:
		program->bindAttribLocation(GENERAL_PROFILE_ATTRIB_POSITION, "vertex_position");
	if( ! this->hasNormalMapping())
		program->bindAttribLocation(GENERAL_PROFILE_ATTRIB_NORMAL, "vertex_normal");
	if(this->hasTextureMapping())
		program->bindAttribLocation(GENERAL_PROFILE_ATTRIB_TEXCOORDS, "vertex_texcoords");

	// - set the frag data locations:
	// -> if "_RENDER_TO_GBUFFER_" is defined (deferred shading):
    if(isSymbolDefined("_RENDER_TO_GBUFFER_", program_id))
	{
		glutil::GPUProgram::Location locations[] = {
			glutil::GPUProgram::Location(GENERAL_PROFILE_FRAG_DATA_POSITION,   "frag_position"),
			glutil::GPUProgram::Location(GENERAL_PROFILE_FRAG_DATA_NORMAL,     "frag_normal"),
			glutil::GPUProgram::Location(GENERAL_PROFILE_FRAG_DATA_DIFFUSE,    "frag_diffuse"),
			glutil::GPUProgram::Location(GENERAL_PROFILE_FRAG_DATA_SPECULAR,   "frag_specular"),
			glutil::GPUProgram::Location(GENERAL_PROFILE_FRAG_DATA_VISIBILITY, "frag_visibility"),
		};

		uint nb_locations = sizeof(locations) / sizeof(glutil::GPUProgram::Location);

		// if "_SHADOW_MAPPING_" is NOT defined, we do NOT use the visibility map, otherwise we do.
		if( ! isSymbolDefined("_SHADOW_MAPPING_", program_id))
		{
			nb_locations--;
		}

		program->bindFragDataLocations(locations, nb_locations);
	}
	// -> if "_RENDER_TO_GBUFFER_" is NOT defined (forward shading):
	else
	{
		program->bindFragDataLocation(GENERAL_PROFILE_FRAG_DATA_COLOR, "frag_color");
	}

	// - link the program:
	bool ok = program->link();
	assert(ok);

	// - set the uniform names:

	// We consider mixing all the possible combinations for uniform names.
	// If one uniform is not available, we just don't care, as the specification
	// of OpenGL allows it (from the documentation of glUniform: "If location is
	// equal to -1, the data passed in will be silently ignored and the
	// specified uniform variable will not be changed.")

	list<string> uniform_names_list;
	uniform_names_list.push_back("view_matrix");
	uniform_names_list.push_back("model_matrix");
	uniform_names_list.push_back("projection_matrix");
	uniform_names_list.push_back("tex_diffuse");
	uniform_names_list.push_back("material_diffuse");
	uniform_names_list.push_back("material_specular");
	uniform_names_list.push_back("tex_normal");
	uniform_names_list.push_back("tex_debug");
	uniform_names_list.push_back("tex_prev_depth_layer");

	stringstream ss;
	for(uint i=0 ; i < NB_MAX_LIGHTS ; i++)
	{
		ss.str("");
		ss << "light_pos_" << i << flush;
		uniform_names_list.push_back(ss.str());

		ss.str("");
		ss << "light_color_" << i << flush;
		uniform_names_list.push_back(ss.str());

		ss.str("");
		ss << "shadow_map_" << i << flush;
		uniform_names_list.push_back(ss.str());

		ss.str("");
		ss << "shadow_matrix_" << i << flush;
		uniform_names_list.push_back(ss.str());
	}

	const string* uniform_names = listToArray(uniform_names_list);
	program->setUniformNames(uniform_names, uniform_names_list.size());
	delete [] uniform_names;

	// - validate the program:
#ifndef NDEBUG
	program->validate();
#endif
}
