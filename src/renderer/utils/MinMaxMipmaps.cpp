// MinMaxMipmaps.cpp

#include "MinMaxMipmaps.h"

#include "MinMaxMipmaps.h"
#include "../../glutil/RAII.h"
#include <sstream>
using namespace std;

// ---------------------------------------------------------------------
// Vertex attrib locations:
#define ATTRIB_POSITION 0

// Fragment output locations are given in GLRaytracerConfig.h
// because they are shared between GLRaytracer and TextureRenderer
// (they render to the same FBO)

// ---------------------------------------------------------------------
MinMaxMipmaps::MinMaxMipmaps(uint target_width, uint target_height)
: target_width(target_width),
  target_height(target_height),
  program(NULL),
  id_vao(0),
  id_vbo(0)
{
}

MinMaxMipmaps::~MinMaxMipmaps()
{
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::setup()
{
	createProgram();
	createVBOAndVAO();
}

void MinMaxMipmaps::cleanup()
{
/*	assert(program != NULL);
	delete program;
	program = NULL;

	assert(id_vbo != 0);
	glDeleteBuffers(1, &id_vbo);
	id_vbo = 0;

	assert(id_vao != 0);
	glDeleteVertexArrays(1, &id_vao);
	id_vao = 0;
*/
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::run()
{
/*	uint texunit = 0;

	// Bind the FBO:
	glutil::BindFramebuffer fbo_binding(gl_raytracer->getFBO());

	// DEBUG
	//glClearColor(1.0, 0.0, 1.0, 0.0);
	//glClear(GL_COLOR_BUFFER_BIT);
	// END DEBUG

	// Set the viewport:
	glutil::SetViewport viewport(offset_x, 0, 1, target_height);

	// Disable the depth test:
	glutil::Disable<GL_DEPTH_TEST> depth_test_state;

	// Bind the GPU program:
	program->use();

	// Bind the textures to the texunits and send the texunits as uniforms:
#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getDebugTex(),     "tex_debug",       program, texunit);
#endif
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getPositionTex(),  "tex_position",    program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getPowerTex(),     "tex_power",       program, texunit);
	BIND_TEX(GL_TEXTURE_RECTANGLE, gl_raytracer->getComingDirTex(), "tex_coming_dir",  program, texunit);

	// Send remaning uniform(s):
	program->sendUniform("offset_x", GLint(offset_x));

	// Bind the VAO and draw
	const uint nb_vertices = 6;
	glBindVertexArray(id_vao);
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
*/
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::createProgram()
{
/*	stringstream ss;
	bool ok = false;

	program = new glutil::GPUProgram(	"media/shaders/raytracer/texture_reduce.vert",
										"media/shaders/raytracer/texture_reduce.frag");

	// Prepare the list of symbols:
	Preprocessor::SymbolList symbols;

	ss.str(""); ss << target_width << flush;
	symbols.push_back(PreprocSym("_TARGET_WIDTH_", ss.str()));

	ss.str(""); ss << target_height << flush;
	symbols.push_back(PreprocSym("_TARGET_HEIGHT_", ss.str()));

#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	symbols.push_back(PreprocSym("_DEBUG_USE_DEBUG_FBO_ATTACHMENT_"));
#endif

	// Set the symbols:
	program->getPreprocessor()->setSymbols(symbols);

	// Compile, attach, set the locations, link
	ok = program->compileAndAttach();
	assert(ok);

	program->bindAttribLocations(ATTRIB_POSITION,  "vertex_position",
								 0,                 NULL);

	program->bindFragDataLocations(
											#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
												FRAG_DATA_DEBUG,      "frag_debug",
											#endif
												FRAG_DATA_POSITION,   "frag_position",
												FRAG_DATA_POWER,      "frag_power",
												FRAG_DATA_COMING_DIR, "frag_coming_dir",
												0,                  NULL
												);

	ok &= program->link();
	assert(ok);

	// Set the uniform names
	list<string> uniform_names;
	uniform_names.push_back("tex_debug");
	uniform_names.push_back("tex_position");
	uniform_names.push_back("tex_power");
	uniform_names.push_back("tex_coming_dir");
	uniform_names.push_back("offset_x");

	program->setUniformNames(uniform_names);

	// Validate
	program->validate();
*/
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::createVBOAndVAO()
{
/*	// Create the VAO:
	glGenVertexArrays(1, &id_vao);

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Vertex and coordinates:
	GLfloat buffer_data[] = {	// Vertex coordinates (x, y)
								-1.0f, -1.0f,	// triangle 1
								+1.0f, -1.0f,
								+1.0f, +1.0f,

								-1.0f, -1.0f,	// triangle 2
								+1.0f, +1.0f,
								-1.0f, +1.0f
							};

	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	const uint nb_vertices = 6;

	// - enable attributes:
	glEnableVertexAttribArray(ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
*/
}
