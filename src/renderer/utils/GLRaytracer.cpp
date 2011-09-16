// GLRaytracer.cpp

#include "GLRaytracer.h"
#include "GLRaytracerConfig.h"

#include "../../Boundaries.h"
#include "../../CommonIndices.h"
#include "GLRaytracer.h"
#include "../../log/Log.h"
#include "../../scene/Light.h"
#include "../../utils/Stringify.h"
#include "GBuffer.h"
#include "BounceMap.h"
#include <iostream>
#include <sstream>

#ifdef USE_DEBUG_TEXTURE
#include "../../utils/TGALoader.h"
#endif
using namespace std;

// ---------------------------------------------------------------------
// Vertex attrib locations:
#define ATTRIB_POSITION  0

// Fragment output locations are given in GLRaytracerConfig.h
// because they are shared between GLRaytracer and TextureRenderer
// (they render to the same FBO)

// ---------------------------------------------------------------------
GLRaytracer::GLRaytracer(uint layers_width,
						 uint layers_height,
						 uint nb_gbuffers)
: layers_width(layers_width),
  layers_height(layers_height),
  target_width(0),
  target_height(0),
  nb_gbuffers(nb_gbuffers),
  id_fbo(0),
  id_debug_target(0),
  id_position(0),
  id_power(0),
  id_coming_dir(0),
  id_vbo(0),
  id_vao(0),
  raytrace_bm_program(NULL),
  id_debug(0)
{
	#ifdef DEBUG_RENDER_TO_SCREEN
		target_width  = layers_width;
		target_height = layers_height;
	#else
		target_width  = FBO_WIDTH;
		target_height = FBO_HEIGHT;
	#endif

	texture_reducer = new TextureReducer(target_width, target_height);
}

GLRaytracer::~GLRaytracer()
{
	delete texture_reducer;
}

void GLRaytracer::setup()
{
	createFBO();
	createVBOAndVAO();
	createRaytraceBMProgram();

	texture_reducer->setup();

	// Load the debug texture:
#ifdef USE_DEBUG_TEXTURE
	TGALoader tga;
	if(tga.loadFile(DEBUG_TEXTURE_FILENAME) == TGA_OK)
	{
		assert(tga.getBpp() == 4);
		id_debug = glutil::createTextureRectRGBA8(tga.getWidth(), tga.getHeight(), (const GLubyte*)(tga.getData()));
	}
#endif
}

void GLRaytracer::cleanup()
{
	// Texture reducer:
	texture_reducer->cleanup();

	// Programs:
	delete raytrace_bm_program;
	raytrace_bm_program = NULL;

	// VBO and VAO:
	glDeleteVertexArrays(1, &id_vao);
	id_vao = 0;

	glDeleteBuffers(1, &id_vbo);
	id_vbo = 0;

	// FBO:
	glDeleteFramebuffers(1, &id_fbo);
	id_fbo = 0;

	glDeleteTextures(1, &id_position);
	id_position = 0;

	glDeleteTextures(1, &id_power);
	id_power = 0;

	glDeleteTextures(1, &id_coming_dir);
	id_coming_dir = 0;

	// Delete the debug texture:
#ifdef USE_DEBUG_TEXTURE
	if(id_debug != 0)
	{
		glDeleteTextures(1, &id_debug);
		id_debug = 0;
	}
#endif
}

// ---------------------------------------------------------------------
void GLRaytracer::run(Light* light,
					  GBuffer **gbuffers,
					  const mat4& eye_proj,
					  const mat4& eye_view,
					  float znear,
					  float zfar)
{
    GL_CHECK();

	// Bind the FBO:
	{
#ifndef DEBUG_RENDER_TO_SCREEN
		glutil::SetViewport viewport(0, 0, target_width, target_height);
		glutil::BindFramebuffer fbo_binding(id_fbo);

		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);
#endif

		// Get some pointers from the light:
		BounceMap* bounce_map = (BounceMap*)(light->getUserData(LIGHT_DATA_BOUNCE_MAP));
		GBuffer* light_gbuffer = (GBuffer*)(light->getUserData(LIGHT_DATA_GBUFFER));

		uint texunit = 0;
		stringstream ss;

		// Disable depth test
		glutil::Disable<GL_DEPTH_TEST> depth_test_state;

		// Bind the program:
		raytrace_bm_program->use();

		// Bind the textures to the texunits and send the texunits as uniforms:
		// - debug texture:
#ifdef USE_DEBUG_TEXTURE
		BIND_TEX(GL_TEXTURE_RECTANGLE, id_debug, "tex_debug", raytrace_bm_program, texunit);
#endif

		// - bounce map:
		BIND_TEX(GL_TEXTURE_RECTANGLE, bounce_map->getTexOutput0(), "bounce_map_0", raytrace_bm_program, texunit);
		BIND_TEX(GL_TEXTURE_RECTANGLE, bounce_map->getTexOutput1(), "bounce_map_1", raytrace_bm_program, texunit);

		// - light GBuffer (positions)
		BIND_TEX(GL_TEXTURE_RECTANGLE, light_gbuffer->getTexPositions(), "tex_light_positions", raytrace_bm_program, texunit);

		// - eye-view depth layers:
		for(uint i=0 ; i < nb_gbuffers ; i++)
		{
			ss.str("");
			ss << "tex_depth_" << i << flush;

			glActiveTexture(GL_TEXTURE0 + texunit);
			glBindTexture(GL_TEXTURE_RECTANGLE, gbuffers[i]->getTexDepth());
			glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_COMPARE_MODE, GL_NONE);
			raytrace_bm_program->sendUniform(ss.str().c_str(), GLint(texunit), Hash::AT_RUNTIME);
			texunit++;
		}

		// Send the bounce map's size:
		raytrace_bm_program->sendUniform("bounce_map_size", GLint(bounce_map->getSize()));

		// Send the far and near plane's distances:
		raytrace_bm_program->sendUniform("znear", znear);
		raytrace_bm_program->sendUniform("zfar",  zfar);

		// Compute and send the matrix used for going from light space to eye space:
		mat4 light_view = light->computeViewMatrix();
		mat4 inv_light_view = glm::inverse(light_view);
		mat4 light_to_eye_matrix = eye_view * inv_light_view;
		raytrace_bm_program->sendUniform("light_to_eye_matrix", light_to_eye_matrix, false);

		// Send the eye projection matrix:
		raytrace_bm_program->sendUniform("eye_proj_matrix", eye_proj, false);

		// Bind the VAO and draw the lines:
		glBindVertexArray(id_vao);

		const uint nb_vertices = 2;

#ifdef DEBUG_NB_LINES
		raytrace_bm_program->sendUniform("x_offset", 0.0f);
		raytrace_bm_program->sendUniform("instance_id_offset", 0);
		glDrawArraysInstanced(GL_LINES, 0, nb_vertices, DEBUG_NB_LINES);
#else
		const uint nb_instances = target_height;
		uint total_nb_lines_to_draw = bounce_map->getSize() * bounce_map->getSize();
		float x_offset_unit = 2.0f / float(target_width);

		for(uint i=0 ; i < total_nb_lines_to_draw ; i += nb_instances)
		{
			float x_offset = x_offset_unit * float(i/nb_instances);
			raytrace_bm_program->sendUniform("x_offset", GLfloat(x_offset));
			raytrace_bm_program->sendUniform("instance_id_offset", GLint(i));

			glDrawArraysInstanced(GL_LINES, 0, nb_vertices, nb_instances);
		}
#endif
	}

	// Texture reduce operation:
#ifndef DEBUG_NO_TEXTURE_REDUCE
	texture_reducer->run(this, 0);
#endif
    
    GL_CHECK();
}

// ---------------------------------------------------------------------
void GLRaytracer::createFBO()
{
	// Setup the textures:
#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	id_debug_target = glutil::createTextureRectRGBA8(FBO_WIDTH, FBO_HEIGHT);
#endif
	id_position     = glutil::createTextureRectRGBAF(FBO_WIDTH, FBO_HEIGHT, true);
	id_power        = glutil::createTextureRectRGBA8(FBO_WIDTH, FBO_HEIGHT);
	id_coming_dir   = glutil::createTextureRectRGBAF(FBO_WIDTH, FBO_HEIGHT, true);

	GL_CHECK();

	// Setup the FBO:
	glGenFramebuffers(1, &id_fbo);
	glutil::BindFramebuffer fbo_binding(id_fbo);

	// - attach the textures:
	uint i = 0;
#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_RECTANGLE, id_debug_target, 0); i++;
#endif
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_RECTANGLE, id_position,     0); i++;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_RECTANGLE, id_power,        0); i++;
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_RECTANGLE, id_coming_dir,   0); i++;

	GL_CHECK();

	// - specify the draw buffers:
#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	static const GLenum draw_buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
		GL_COLOR_ATTACHMENT3,
	};
#else
	static const GLenum draw_buffers[] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1,
		GL_COLOR_ATTACHMENT2,
	};
#endif
	glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);

	// Check the FBO:
	GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
		logSuccess("FBO creation");
	else
		logError("FBO not complete");
}

// ---------------------------------------------------------------------
void GLRaytracer::createVBOAndVAO()
{
	// Create the VAO:
	glGenVertexArrays(1, &id_vao);

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Vertex coordinates:
	GLfloat buffer_data[] = {
								-1.0f, -1.0f,
								+1.0f, -1.0f
							};


	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	// - enable attributes:
	glEnableVertexAttribArray(ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
}

// ---------------------------------------------------------------------
void GLRaytracer::createRaytraceBMProgram()
{
	stringstream ss;
	bool ok = false;

	raytrace_bm_program = new glutil::GPUProgram("media/shaders/raytracer/raytrace.vert",
												 "media/shaders/raytracer/raytrace.frag");

	// Prepare the list of symbols:
	Preprocessor::SymbolList symbols;
	symbols.push_back(PreprocSym("_FROM_BOUNCE_MAP_"));

	ss.str(""); ss << target_width << ".0" << flush;
	symbols.push_back(PreprocSym("_TARGET_WIDTH_", ss.str()));

	ss.str(""); ss << target_height << ".0" << flush;
	symbols.push_back(PreprocSym("_TARGET_HEIGHT_", ss.str()));

	ss.str(""); ss << layers_width << ".0" << flush;
	symbols.push_back(PreprocSym("_LAYERS_WIDTH_", ss.str()));

	ss.str(""); ss << layers_height << ".0" << flush;
	symbols.push_back(PreprocSym("_LAYERS_HEIGHT_", ss.str()));

	symbols.push_back(PreprocSym("_NB_DEPTH_LAYERS_", nb_gbuffers));

#ifdef DEBUG_USE_EYE_SPACE_LINES
	symbols.push_back(PreprocSym("_DEBUG_USE_EYE_SPACE_LINES_"));
#endif

#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	symbols.push_back(PreprocSym("_DEBUG_USE_DEBUG_FBO_ATTACHMENT_"));
#endif

#ifdef USE_DEBUG_TEXTURE
	symbols.push_back(PreprocSym("_DEBUG_TEXTURE_"));
#endif

	// Set the symbols:
	raytrace_bm_program->getPreprocessor()->setSymbols(symbols);

	// Compile, attach, set the locations, link
	ok = raytrace_bm_program->compileAndAttach();
	assert(ok);

	raytrace_bm_program->bindAttribLocations(ATTRIB_POSITION,  "vertex_position",
											 0,                 NULL);

	raytrace_bm_program->bindFragDataLocations(
											#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
												FRAG_DATA_DEBUG,      "frag_debug",
											#endif
												FRAG_DATA_POSITION,   "frag_position",
												FRAG_DATA_POWER,      "frag_power",
												FRAG_DATA_COMING_DIR, "frag_coming_dir",
												0,                  NULL
												);

	ok &= raytrace_bm_program->link();
	assert(ok);

	// Set the uniform names
	list<string> uniform_names;
	uniform_names.push_back("bounce_map_0");
	uniform_names.push_back("bounce_map_1");
	uniform_names.push_back("bounce_map_size");
	uniform_names.push_back("tex_light_positions");
	uniform_names.push_back("light_to_eye_matrix");
	uniform_names.push_back("eye_proj_matrix");
	uniform_names.push_back("tex_debug");
	uniform_names.push_back("znear");
	uniform_names.push_back("zfar");
	uniform_names.push_back("x_offset");
	uniform_names.push_back("instance_id_offset");

	for(uint i=0 ; i < NB_MAX_DEPTH_LAYERS ; i++)
	{
		ss.str("");
		ss << "tex_depth_" << i << flush;
		uniform_names.push_back(ss.str());
	}

	raytrace_bm_program->setUniformNames(uniform_names);

	// Validate
	raytrace_bm_program->validate();
}
