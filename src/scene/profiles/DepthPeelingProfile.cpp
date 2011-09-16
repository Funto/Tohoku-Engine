// DepthPeelingProfile.cpp

#include "DepthPeelingProfile.h"
#include "../../ShaderLocations.h"

DepthPeelingProfile::DepthPeelingProfile()
{
}

DepthPeelingProfile::~DepthPeelingProfile()
{
}

void DepthPeelingProfile::onNewProgram(glutil::GPUProgram *program)
{
	// On creation of a new program:
	// - set the attrib locations:
	program->bindAttribLocation(DEPTH_PEELING_ATTRIB_POSITION, "vertex_position");

	if( this->hasNormalMapping())
		program->bindAttribLocation(DEPTH_PEELING_ATTRIB_TEXCOORDS, "vertex_texcoords");
	else
		program->bindAttribLocation(DEPTH_PEELING_ATTRIB_NORMAL,    "vertex_normal");

	// - set the frag data locations:
	program->bindFragDataLocation(DEPTH_PEELING_FRAG_DATA_NORMAL, "frag_color");

	// - link the program:
	bool ok = program->link();
	assert(ok);

	// - set the possible uniform names:
	program->setUniformNames("view_matrix",
							 "model_matrix",
							 "projection_matrix",
							 "tex_normal",
							 "tex_prev_depth_layer",
							 NULL);

	// - validate the program:
#ifndef NDEBUG
	program->validate();
#endif
}
