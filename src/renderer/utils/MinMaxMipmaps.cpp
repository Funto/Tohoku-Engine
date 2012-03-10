// MinMaxMipmaps.cpp

#include "MinMaxMipmaps.h"

#include "MinMaxMipmaps.h"
#include "../../glutil/glutil.h"
#include "../../ShaderLocations.h"
#include <sstream>
using namespace std;

// ---------------------------------------------------------------------
MinMaxMipmaps::MinMaxMipmaps(uint target_width, uint target_height)
: target_width(target_width),
  target_height(target_height),
  program_first(NULL),
  program_next(NULL),
  id_vao(0),
  id_vbo(0),
  nb_layers(0),
  id_fbo(NULL),
  id_min_max_tex(NULL)
{
}

MinMaxMipmaps::~MinMaxMipmaps()
{
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::setup()
{
	createFBOs();
	createPrograms();
	createVBOAndVAO();
}

void MinMaxMipmaps::cleanup()
{
	// Programs
	assert(program_first != NULL);
	delete program_first;
	program_first = NULL;
	
	assert(program_next != NULL);
	delete program_next;
	program_next = NULL;
	
	// FBOs and textures
	assert(nb_layers > 0);
	assert(id_fbo != NULL);
	assert(id_min_max_tex != NULL);
	
	glDeleteFramebuffers(nb_layers, id_fbo);
	glDeleteTextures(nb_layers, id_min_max_tex);
	nb_layers = 0;
	delete [] id_fbo;
	id_fbo = NULL;
	
	delete [] id_min_max_tex;
	id_min_max_tex = NULL;
	
	// VBO/VAO
	assert(id_vbo != 0);
	glDeleteBuffers(1, &id_vbo);
	id_vbo = 0;

	assert(id_vao != 0);
	glDeleteVertexArrays(1, &id_vao);
	id_vao = 0;
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::run(GLuint id_tex_position)
{
	uint w = target_width;
	uint h = target_height;
	
	// Disable the depth test:
	glutil::Disable<GL_DEPTH_TEST> depth_test_state;
	
	for(uint i=0 ; i < nb_layers ; i++)
	{
		glutil::BindFramebuffer fbo_binding(id_fbo[i]);
		
		glutil::SetViewport viewport(0, 0, w, h);
		
		if(i == 0)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			
			program_first->use();
			
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, id_tex_position);
			program_first->sendUniform("tex_position", 0);
		}
		else
		{
			program_next->use();
			
			const GLuint id_tex_prev_layer = id_min_max_tex[i-1];
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_RECTANGLE, id_tex_prev_layer);
			program_next->sendUniform("tex_prev_layer", 0);
			
			program_next->sendUniform("target_size", vec2(w, h));
		}
	
		// Bind the VAO and draw
		const uint nb_vertices = 6;
		glBindVertexArray(id_vao);
		glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
		
		w >>= 1;
		h >>= 1;
	}
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::createFBOs()
{
	// Compute nb_layers
	uint w = target_width;
	uint h = target_height;
	nb_layers = 0;
	while(w >= 16)	// up to 16x9 - TODO: common denominator
	{
		nb_layers++;
		w >>= 1;
		h >>= 1;
	}
	
	id_fbo = new GLuint[nb_layers];
	id_min_max_tex = new GLuint[nb_layers];
	
	glGenFramebuffers(nb_layers, id_fbo);
	
	// Create all layers
	w = target_width;
	h = target_height;
	for(uint i=0 ; i < nb_layers ; i++)
	{
		// Setup the textures:
		id_min_max_tex[i] = glutil::createTextureRectRGBAF(w, h, true);
	
		GL_CHECK();
	
		// Setup a FBO:
		glutil::BindFramebuffer fbo_binding(id_fbo[i]);
	
		// - attach the texture:
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE, id_min_max_tex[i], 0);
	
		GL_CHECK();
	
		// - specify the draw buffers :
		static const GLenum draw_buffers[] = {
			GL_COLOR_ATTACHMENT0,
		};
		glDrawBuffers(sizeof(draw_buffers) / sizeof(GLenum), draw_buffers);
	
		// Check the FBO:
		GLenum fbo_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	
		if(fbo_status == GL_FRAMEBUFFER_COMPLETE)
			logSuccess("FBO creation");
		else
			logError("FBO not complete");
		
		w >>= 1;
		h >>= 1;
	}
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::createPrograms()
{
	stringstream ss;
	bool ok = false;

	// First layer program
	{
		program_first = new glutil::GPUProgram(	"media/shaders/min_max_mipmaps_first.vert",
												"media/shaders/min_max_mipmaps_first.frag");
	
		// Prepare the list of symbols:
		Preprocessor::SymbolList symbols;
	
		ss.str(""); ss << target_width << flush;
		symbols.push_back(PreprocSym("_TARGET_WIDTH_", ss.str()));
	
		ss.str(""); ss << target_height << flush;
		symbols.push_back(PreprocSym("_TARGET_HEIGHT_", ss.str()));
	
		// Set the symbols:
		program_first->getPreprocessor()->setSymbols(symbols);
	
		// Compile, attach, set the locations, link
		ok = program_first->compileAndAttach();
		assert(ok);
	
		program_first->bindAttribLocations(MIN_MAX_MIPMAPS_ATTRIB_POSITION, "vertex_position",
		                                   0, NULL);
	
		program_first->bindFragDataLocations(	MIN_MAX_MIPMAPS_FRAG_DATA_OUTPUT, "frag_output",
		                                     0, NULL);
	
		ok &= program_first->link();
		assert(ok);
	
		// Set the uniform names
		list<string> uniform_names;
		uniform_names.push_back("tex_position");
	
		program_first->setUniformNames(uniform_names);
	
		// Validate
		program_first->validate();
	}
	
	// Next layers program
	{
		program_next = new glutil::GPUProgram(	"media/shaders/min_max_mipmaps_next.vert",
												"media/shaders/min_max_mipmaps_next.frag");
	
		// Prepare the list of symbols:
		Preprocessor::SymbolList symbols;
	
		// Set the symbols:
		program_next->getPreprocessor()->setSymbols(symbols);
	
		// Compile, attach, set the locations, link
		ok = program_next->compileAndAttach();
		assert(ok);
	
		program_next->bindAttribLocations(MIN_MAX_MIPMAPS_ATTRIB_POSITION, "vertex_position",
		                                  0, NULL);
	
		program_next->bindFragDataLocations(MIN_MAX_MIPMAPS_FRAG_DATA_OUTPUT, "frag_output",
		                                    0, NULL);
	
		ok &= program_next->link();
		assert(ok);
	
		// Set the uniform names
		list<string> uniform_names;
		uniform_names.push_back("tex_prev_layer");
		uniform_names.push_back("target_size");
	
		program_next->setUniformNames(uniform_names);
	
		// Validate
		program_next->validate();
	}
}

// ---------------------------------------------------------------------
void MinMaxMipmaps::createVBOAndVAO()
{
	// Create the VAO:
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
	glEnableVertexAttribArray(MIN_MAX_MIPMAPS_ATTRIB_POSITION);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(MIN_MAX_MIPMAPS_ATTRIB_POSITION, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
}
