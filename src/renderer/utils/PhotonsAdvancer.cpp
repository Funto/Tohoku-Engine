// PhotonsAdvancer.cpp

#include "PhotonsAdvancer.h"

PhotonsAdvancer::PhotonsAdvancer(uint layers_width,
                                 uint layers_height,
								 uint nb_gbuffers)
    : layers_width(layers_width),
	  layers_height(layers_height),
	  target_width(0),
	  target_height(0),
	  nb_gbuffers(nb_gbuffers),
	  id_fbo(0),
	  id_position(0),
	  id_vbo(0),
	  id_vao(0),
	  advance_photons_program(NULL)
{
}

PhotonsAdvancer::~PhotonsAdvancer()
{
}

void PhotonsAdvancer::setup()
{
}

void PhotonsAdvancer::cleanup()
{
}

void PhotonsAdvancer::run(Light* light,
						 GBuffer** gbuffers,
						 const mat4& eye_proj,
						 const mat4& eye_view,
						 float znear,
						 float zfar)
{
}

void PhotonsAdvancer::createFBO()
{
}

void PhotonsAdvancer::createVBOAndVAO()
{
}

void PhotonsAdvancer::createAdvancePhotonsProgram()
{
}
