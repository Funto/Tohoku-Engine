// GBufferRenderer.cpp

#include "../../Common.h"
#include "../../CommonIndices.h"
#include "../../Boundaries.h"
#include "../../utils/StdListManip.h"
#include "../../scene/Camera.h"
#include "../../scene/Light.h"
#include "../../glutil/RAII.h"
#include "GBufferRenderer.h"
#include "GBuffer.h"
#include "ShadowMap.h"
#include <string>
#include <sstream>
using namespace std;

// Locations for the render_from_gbuffer program:
#define ATTRIB_POSITION  0
#define ATTRIB_TEXCOORDS 1

#define FRAG_DATA_COLOR      0
#define FRAG_DATA_PIXEL_DONE 1

// ---------------------------------------------------------------------
GBufferRenderer::GBufferRenderer(uint width,
								 uint height)
: width(width),
  height(height),
  program(NULL),
  id_vao(0),
  id_vbo(0)
{
}

GBufferRenderer::~GBufferRenderer()
{
}

// ---------------------------------------------------------------------
void GBufferRenderer::load(const Preprocessor::SymbolList& preproc_syms)
{
	// Create the program for rendering to the GBuffer:
	createProgram(preproc_syms);

	// Create the VBO and the VAO for the quad:
	createVBOAndVAO();
}

void GBufferRenderer::unload()
{
	assert(program != NULL);
	delete program;
	program = NULL;

	assert(id_vbo != 0);
	glDeleteBuffers(1, &id_vbo);
	id_vbo = 0;

	assert(id_vao != 0);
	glDeleteVertexArrays(1, &id_vao);
	id_vao = 0;
}

// ---------------------------------------------------------------------
// Binds stuff, and lets the user know which one is the first texunit he can use.
void GBufferRenderer::bind(GBuffer* gbuffer,
						   Camera* camera,
						   Light** lights,
						   uint nb_lights,
						   bool bind_shadow_maps,
						   const vec3& back_color,
						   uint* first_valid_texunit)
{
	stringstream ss;
	uint texunit = 0;

	// Start using the program:
	program->use();

	// Set the background color:
	program->sendUniform("back_color", back_color);

	// Bind the textures of the GBuffer to the texunits and send
	// the corresponding uniforms:
	// - positions:
	glActiveTexture(GL_TEXTURE0 + texunit);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexPositions());
	program->sendUniform("tex_positions", GLint(texunit));
	texunit++;

	// - normals:
	glActiveTexture(GL_TEXTURE0 + texunit);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexNormals());
	program->sendUniform("tex_normals",   GLint(texunit));
	texunit++;

	// - diffuse:
	glActiveTexture(GL_TEXTURE0 + texunit);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexDiffuse());
	program->sendUniform("tex_diffuse",   GLint(texunit));
	texunit++;

	// - specular:
	glActiveTexture(GL_TEXTURE0 + texunit);
	glBindTexture(GL_TEXTURE_RECTANGLE, gbuffer->getTexSpecular());
	program->sendUniform("tex_specular",  GLint(texunit));
	texunit++;

	// Bias matrix, used for shadow mapping. Lets us switch from the range [-1, 1] to [0, 1]
	// (0.5*x + 0.5)
	mat4 bias_matrix = mat4(vec4(0.5, 0.0, 0.0, 0.0),
							vec4(0.0, 0.5, 0.0, 0.0),
							vec4(0.0, 0.0, 0.5, 0.0),
							vec4(0.5, 0.5, 0.5, 1.0));

	// Compute the view matrix:
	mat4 view_matrix = camera->computeViewMatrix();

	// Inverse the view matrix (for shadows):
	mat4 inv_view_matrix = glm::inverse(view_matrix);


	// For each light:
	for(uint i=0 ; i < nb_lights ; i++)
	{
		Light* l = lights[i];

		// Compute the light position in eye space and send it as a uniform:
		vec3 eye_space_light_pos = vec3(view_matrix * vec4(l->getPosition(), 1.0f));

		ss.str("");
		ss << "eye_space_light_pos_" << i << flush;
		program->sendUniform(ss.str().c_str(), eye_space_light_pos, Hash::AT_RUNTIME);

		ss.str("");
		ss << "light_color_" << i << flush;
		program->sendUniform(ss.str().c_str(), l->getColor(), Hash::AT_RUNTIME);

		// Shadow mapping without visibility maps (computed on-the-fly):
		if(bind_shadow_maps)
		{
			ShadowMap* shadow_map = (ShadowMap*)(l->getUserData(LIGHT_DATA_SHADOW_MAP));

			// Shadow mapping without visibility maps (computed during the rendering of
			// the full-screen quad):

			// Compute the shadow matrix:
			// inverse of view:   eye space -> world space
			// light view:        world space -> light space
			// light proj:        light space -> light screen space
			// bias matrix:       switches from [-1, 1] to [0, 1]
			mat4 shadow_matrix = bias_matrix * l->computeProjectionMatrix() * l->computeViewMatrix() * inv_view_matrix ;

			// Send the shadow matrix as a uniform:
			ss.str("");
			ss << "shadow_matrix_" << i << flush;
			program->sendUniform(ss.str().c_str(), shadow_matrix, false, Hash::AT_RUNTIME);

			// Bind the shadow map to the corresponding texture unit:
			glActiveTexture(GL_TEXTURE0 + texunit);
			glBindTexture(GL_TEXTURE_2D, shadow_map->getTexDepth());

			// Set the parameters for shadow comparison and disable any possible mip-mapping:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,       GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,       GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,   GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,   GL_NEAREST);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

			// Send the texunit as an uniform:
			ss.str("");
			ss << "shadow_map_" << i << flush;
			program->sendUniform(ss.str().c_str(), GLint(texunit), Hash::AT_RUNTIME);

			texunit++;
		}
	}

	// If asked, return the number of the first valid texunit the user can use:
	if(first_valid_texunit != NULL)
		*first_valid_texunit = texunit;
}

// IMPORTANT: bind() has to be called before render()!
void GBufferRenderer::render()
{
	// Disable depth test
	glutil::Disable<GL_DEPTH_TEST> depth_test_state;

	// Bind the VAO and draw
	const uint nb_vertices = 6;
	glBindVertexArray(id_vao);
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
}

// ---------------------------------------------------------------------
void GBufferRenderer::createProgram(const Preprocessor::SymbolList& additional_preproc_syms)
{
	program = new glutil::GPUProgram("media/shaders/render_from_gbuffer.vert",
									 "media/shaders/render_from_gbuffer.frag");
	program->getPreprocessor()->setSymbols(additional_preproc_syms);

	bool ok = program->compileAndAttach();
	assert(ok);

	program->bindAttribLocations(ATTRIB_POSITION,  "vertex_position",
								 ATTRIB_TEXCOORDS, "vertex_texcoords",
								 0,                NULL);
	program->bindFragDataLocation(FRAG_DATA_COLOR, "frag_color");

	if(program->getPreprocessor()->hasOriginalSymbol("_MARK_PIXELS_DONE_"))
		program->bindFragDataLocation(FRAG_DATA_PIXEL_DONE, "frag_pixel_done");

	ok &= program->link();
	assert(ok);

	// Set the uniform names:
	list<string> uniform_names_list;
	uniform_names_list.push_back("tex_positions");
	uniform_names_list.push_back("tex_normals");
	uniform_names_list.push_back("tex_diffuse");
	uniform_names_list.push_back("tex_specular");
	uniform_names_list.push_back("visibility_map");
	uniform_names_list.push_back("back_color");
	uniform_names_list.push_back("tex_debug");
	uniform_names_list.push_back("tex_pixels_done");

	stringstream ss;
	for(uint i=0 ; i < NB_MAX_LIGHTS ; i++)
	{
		ss.str("");
		ss << "eye_space_light_pos_" << i << flush;
		uniform_names_list.push_back(ss.str());

		ss.str("");
		ss << "shadow_matrix_" << i << flush;
		uniform_names_list.push_back(ss.str());

		ss.str("");
		ss << "shadow_map_" << i << flush;
		uniform_names_list.push_back(ss.str());

		ss.str("");
		ss << "light_color_" << i << flush;
		uniform_names_list.push_back(ss.str());
	}

	const string* uniform_names = listToArray(uniform_names_list);
	program->setUniformNames(uniform_names, uniform_names_list.size());
	delete [] uniform_names;

	program->validate();
}

// ---------------------------------------------------------------------
void GBufferRenderer::createVBOAndVAO()
{
	GLfloat max_x = GLfloat(width);
	GLfloat max_y = GLfloat(height);

	// Create the VAO:
	glGenVertexArrays(1, &id_vao);

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Vertex and texture coordinates:
	GLfloat buffer_data[] = {	// Vertex coordinates (x, y)
								-1.0f, -1.0f,	// triangle 1
								+1.0f, -1.0f,
								+1.0f, +1.0f,

								-1.0f, -1.0f,	// triangle 2
								+1.0f, +1.0f,
								-1.0f, +1.0f,

								// Texture coordinates (u, v)
								0.0f,  0.0f,	// triangle 1
								max_x, 0.0f,
								max_x, max_y,

								0.0f,  0.0f,	// triangle 2
								max_x, max_y,
								0.0f,  max_y};

	const GLsizeiptr buffer_size = sizeof(buffer_data);

	// Create the buffer and put the data into it:
	glGenBuffers(1, &id_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo);

	glBufferData(GL_ARRAY_BUFFER, buffer_size, (const GLvoid*)buffer_data, GL_STATIC_DRAW);

	const uint nb_vertices = 6;

	// - enable attributes:
	glEnableVertexAttribArray(ATTRIB_POSITION);
	glEnableVertexAttribArray(ATTRIB_TEXCOORDS);

	// - vertices pointer:
	ptrdiff_t offset = NULL;
	glVertexAttribPointer(ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;

	// - texcoords pointer:
	glVertexAttribPointer(ATTRIB_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);
	offset += sizeof(GLfloat)*nb_vertices*2;
}
