// glutil.cpp

#include "glutil.h"
#include <GL/glfw.h>
#include "../log/Log.h"

#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
#include <cassert>
using namespace std;

//#define ENABLE_GPU_MEMORY_PRINT

// ---------------------------------------------------------------------
#define DEBUG_TEXTURE_ATTRIB_POSITION  0
#define DEBUG_TEXTURE_ATTRIB_TEXCOORDS 1

#define DEBUG_TEXTURE_FRAG_DATA_COLOR 0

// ---------------------------------------------------------------------
#define DEBUG_UNICOLOR_ATTRIB_POSITION 0
#define DEBUG_UNICOLOR_ATTRIB_COLOR 1

#define DEBUG_UNICOLOR_FRAG_DATA_COLOR 0

// ---------------------------------------------------------------------
namespace glutil
{

// GPUProgram used by displayTexture2D()
static GPUProgram* debug_texture2D_program = NULL;

// GPUProgram used by displayTextureRect()
static GPUProgram* debug_texture_rect_program = NULL;

// GPUProgram used by drawPoint() and drawLine()
static GPUProgram* debug_unicolor_program = NULL;

// VAO used for displaying anything with glutil.
// We use it as one would use the default vertex array object
// (i.e its data is changed when we call drawLine(), drawPoint(), etc.)
static GLuint id_vao = 0;

// VBO used for displaying a quad, by displayTexture2D() and displayTextureRect()
static GLuint id_vbo_quad = 0;

// VBO used for displaying a point, by drawPoint()
static GLuint id_vbo_point = 0;

// VBO used for displaying a point, by drawLine()
static GLuint id_vbo_line = 0;

// Initialize glutil
void init()
{
	// Create the VAO used for all display functions of glutil:
	glGenVertexArrays(1, &id_vao);
	glBindVertexArray(id_vao);

	// Create the VBO used for displaying quads:
	glGenBuffers(1, &id_vbo_quad);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_quad);

	GLsizeiptr total_size = sizeof(GLfloat)*6*(2+2);	// 6 vertices, each one having (x, y) and (u, v).
	glBufferData(GL_ARRAY_BUFFER, total_size, NULL, GL_STREAM_DRAW);

	// Create the VBO for displaying points (vertex specification: x, y, z, r, g, b):
	glGenBuffers(1, &id_vbo_point);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_point);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*(3+3), NULL, GL_STREAM_DRAW);

	// Create the VBO for displaying lines (vertex specification: x1, y1, z1, x2, y2, z2, r1, g1, b1, r2, g2, b2):
	glGenBuffers(1, &id_vbo_line);
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_line);
	glBufferData(GL_ARRAY_BUFFER, 2*sizeof(GLfloat)*(3+3), NULL, GL_STREAM_DRAW);

	// Create the GPU programs:
	// - debug_unicolor_program:
	{
		debug_unicolor_program = new GPUProgram("media/shaders/debug/unicolor.vert",
											 "media/shaders/debug/unicolor.frag");
		bool ok = debug_unicolor_program->compileAndAttach();
		assert(ok);

		debug_unicolor_program->bindAttribLocation(DEBUG_UNICOLOR_ATTRIB_POSITION, "vertex_position");
		debug_unicolor_program->bindAttribLocation(DEBUG_UNICOLOR_ATTRIB_COLOR,    "vertex_color");
		debug_unicolor_program->bindFragDataLocation(DEBUG_UNICOLOR_FRAG_DATA_COLOR, "frag_color");

		ok &= debug_unicolor_program->link();
		assert(ok);

		debug_unicolor_program->setUniformNames("proj_matrix", "view_matrix", NULL);

		debug_unicolor_program->validate();
	}

	// - debug_texture2D_program and debug_texture_rect_program:
	for(uint i=0 ; i < 2 ; i++)
	{
		GPUProgram* p = NULL;
		if(i == 0)
			p = debug_texture2D_program    = new GPUProgram(	"media/shaders/debug/texture2D.vert",
																"media/shaders/debug/texture2D.frag");
		else
			p = debug_texture_rect_program = new GPUProgram(	"media/shaders/debug/texture_rect.vert",
																"media/shaders/debug/texture_rect.frag");

		bool ok = p->compileAndAttach();
		assert(ok);

		p->bindAttribLocation(DEBUG_TEXTURE_ATTRIB_POSITION,  "vertex_position");
		p->bindAttribLocation(DEBUG_TEXTURE_ATTRIB_TEXCOORDS, "vertex_texcoords");
		p->bindFragDataLocation(DEBUG_TEXTURE_FRAG_DATA_COLOR, "frag_color");

		ok &= p->link();
		assert(ok);

		p->setUniformNames("texunit", NULL);
		p->validate();
	}
}

// Shutdown glutil
void shutdown()
{
	// Free the VBO used for displaying quads:
	if(id_vbo_quad != 0)
	{
		glDeleteBuffers(1, &id_vbo_quad);
		id_vbo_quad = 0;
	}

	// Free the VBO used for displaying points:
	if(id_vbo_point != 0)
	{
		glDeleteBuffers(1, &id_vbo_point);
		id_vbo_point = 0;
	}

	// Free the VBO used for displaying lines:
	if(id_vbo_line != 0)
	{
		glDeleteBuffers(1, &id_vbo_line);
		id_vbo_line = 0;
	}

	// Free the VAO:
	if(id_vao != 0)
	{
		glDeleteVertexArrays(1, &id_vao);
		id_vao = 0;
	}

	// Free GPU programs:
	delete debug_texture2D_program;
	debug_texture2D_program = NULL;

	delete debug_texture_rect_program;
	debug_texture_rect_program = NULL;

	delete debug_unicolor_program;
	debug_unicolor_program = NULL;
}

// Get a string describing an OpenGL error.
// Returns NULL if unknown error.
struct ErrorString
{
	GLenum error;
	const char* error_string;
};

const char* getGLErrorString(GLenum error)
{
	static ErrorString errors[] = {
		{ GL_NO_ERROR,                      "GL_NO_ERROR"     },
		{ GL_INVALID_ENUM,                  "GL_INVALID_ENUM" },
		{ GL_INVALID_VALUE,                 "GL_INVALID_VALUE"},
		{ GL_INVALID_OPERATION,             "GL_INVALID_OPERATION"},
		{ GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"},
		{ GL_OUT_OF_MEMORY,                 "GL_OUT_OF_MEMORY"}
	};

	static const unsigned nb_errors = sizeof(errors) / sizeof(ErrorString);

	for(unsigned i=0 ; i < nb_errors ; i++)
	{
		if(errors[i].error == error)
			return errors[i].error_string;
	}
	return NULL;
}

// Check OpenGL errors
bool checkGLErrors(const char* file, unsigned line, const char* function)
{
	bool reported_error = false;

#if MAX_GL_ERRORS > 0
	static uint nb_reported_errors = 0;

	if(nb_reported_errors < MAX_GL_ERRORS)
	{
#endif
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			const char* s = getGLErrorString(error);
			string error_string = "";

			if(s != NULL)
				error_string = s;
			else
			{
				stringstream ss;
				ss << "unknown error (code value: 0x" << hex << int(error) << ")";
				error_string = ss.str();
			}

			Log::write(LOG_ERROR, file, line, function, "OpenGL error: ", error_string);
			reported_error = true;

#if MAX_GL_ERRORS > 0
			nb_reported_errors++;
			if(nb_reported_errors == MAX_GL_ERRORS)
				logWarn("[reached limit of ", MAX_GL_ERRORS, " OpenGL errors, stop reporting]");
#endif
		}
#if MAX_GL_ERRORS > 0
	}
#endif

	return reported_error;
}

// Print information on the GPU memory.
// Extension used: GL_NVX_gpu_memory_info
void printGPUMemoryInfo(const char* msg)
{
#ifdef ENABLE_GPU_MEMORY_PRINT
	static const GLenum GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX          = 0x9047;
	static const GLenum GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX    = 0x9048;
	static const GLenum GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX  = 0x9049;
	static const GLenum GPU_MEMORY_INFO_EVICTION_COUNT_NVX            = 0x904A;
	static const GLenum GPU_MEMORY_INFO_EVICTED_MEMORY_NVX            = 0x904B;

	uint nb_extensions = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, (GLint*)(&nb_extensions));

	// Check if the extension is present:
	bool found = false;
	for(uint i=0 ; i < nb_extensions ; i++)
		if(strcmp((const char*)(glGetStringi(GL_EXTENSIONS, i)), "GL_NVX_gpu_memory_info") == 0)
			found = true;

	if(found)
	{
		if(msg != NULL)
			logDebug(msg);

		GLint i=0;
		uint nb_mb = 0;
		uint nb_kb = 0;

#define PRINT_MEM(sym, msg)	\
		glGetIntegerv(sym, &i);	\
		nb_mb = uint(i/1024);	\
		nb_kb = uint(i) - nb_mb*1024;	\
		logDebug(msg, ": ", nb_mb, "Mb ", nb_kb, "Kb")

		PRINT_MEM(GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX,			"dedicated memory");
		PRINT_MEM(GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX,	"total available memory");
		PRINT_MEM(GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, "current available video memory");
		PRINT_MEM(GPU_MEMORY_INFO_EVICTED_MEMORY_NVX,			"evicted memory");

		glGetIntegerv(GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &i);
		logDebug("eviction count: ", i);
	}
	else
		logError("GL_NVX_gpu_memory_info is not supported, can not print GPU memory information");
#endif
}

// Load a source file in a shader
bool loadShaderSource(GLuint id_shader, const char* file_path)
{
	ifstream f(file_path);
	if(!f)
	{
		logError("impossible to read the file \"", file_path, "\"");
		return false;
	}

	int filesize = 0;
	GLchar* buf = NULL;

	// Get the file size
	f.seekg(0, ios::end);
	filesize = f.tellg();

	// Allocate memory for the text and get the file's contents
	buf = new GLchar[filesize+1];
	memset(buf, 0, filesize);
	f.seekg(0, ios::beg);
	f.read(buf, filesize);
	buf[filesize] = '\0';

	f.close();

	glShaderSource(id_shader, 1, (const GLchar**)(&buf), NULL);

	delete [] buf;

	return true;
}

// Print the log of a shader compilation
void printShaderLog(GLuint id_shader, const char* shader_type)
{
	GLchar* buf = NULL;
	GLint len = 0;

	glGetShaderiv(id_shader, GL_INFO_LOG_LENGTH, &len);

	buf = new GLchar[len];
	glGetShaderInfoLog(id_shader, len, &len, buf);

	cout << shader_type << " log:" << endl << buf << endl;
	delete [] buf;
}

// Print the log of linking a program
void printProgramLog(GLuint id_program)
{
	GLchar* buf = NULL;
	GLint len = 0;

	glGetProgramiv(id_program, GL_INFO_LOG_LENGTH, &len);

	buf = new GLchar[len];
	glGetProgramInfoLog(id_program, len, &len, buf);

	cout << "program log: " << endl << buf << endl;

	delete [] buf;
}

// Check if a program is valid
bool checkProgramValid(GLuint id_program)
{
	glValidateProgram(id_program);

	GLint status;
	glGetProgramiv(id_program, GL_VALIDATE_STATUS, &status);

	if(status == GL_TRUE)
		logSuccess("program validated");
	else
		logFailed("program not validated");

	return bool(status);
}

// Display a quad made of 2 triangles for debugging textures
void displayTexture2D(GLuint id_texture, unsigned pos_x, unsigned pos_y)
{
	Disable<GL_DEPTH_TEST> depth_test_state;

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Texture :
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id_texture);

	// Texture parameters:
	TexParameter<GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE> tex_compare_mode;

	// Setup shader and its uniforms :
	assert(debug_texture2D_program != NULL);
	debug_texture2D_program->use();
	debug_texture2D_program->sendUniform("texunit", 0);

	// Compute position and size:
	GLfloat x = 2.0f * (GLfloat(pos_x) * DEBUG_QUAD_SIZE / 800.0f - 0.5f);
	GLfloat y = 2.0f * (GLfloat(pos_y) * DEBUG_QUAD_SIZE / 600.0f - 0.5f);
	GLfloat w = 2.0f * DEBUG_QUAD_SIZE / 800.0f;
	GLfloat h = 2.0f * DEBUG_QUAD_SIZE / 600.0f;

	// Draw :
	GLfloat vertices[] = {x  , y,
						  x+w, y,
						  x+w, y+h,

						  x,   y,
						  x+w, y+h,
						  x,   y+h};
	const GLsizei nb_vertices = sizeof(vertices) / (2*sizeof(float));

	GLfloat texcoords[nb_vertices*2] = {	0.0f, 0.0f,
											1.0f, 0.0f,
											1.0f, 1.0f,

											0.0f, 0.0f,
											1.0f, 1.0f,
											0.0f, 1.0f};

	// - bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_quad);

	// - enable the vertex attributes arrays
	glEnableVertexAttribArray(DEBUG_TEXTURE_ATTRIB_POSITION);
	glEnableVertexAttribArray(DEBUG_TEXTURE_ATTRIB_TEXCOORDS);

	// - put data in the VBO and set the vertex attrib offsets:
	GLintptr offset = 0;
	GLsizeiptr size = 0;

	// - => vertices:
	offset = 0;
	size = sizeof(GLfloat)*nb_vertices*2;
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices);
	glVertexAttribPointer(DEBUG_TEXTURE_ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);

	// - => texcoords:
	offset += size;
	size = sizeof(GLfloat)*nb_vertices*2;
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, texcoords);
	glVertexAttribPointer(DEBUG_TEXTURE_ATTRIB_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);

	// - draw
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
}

// Display a quad made of 2 triangles for debugging rectangle textures.
// (pos_x, pos_y) has the same meaning as for displayTexture2D().
// (tex_width, tex_height) is the size of the texture rectangle.
// If original_size == true, the quad will have the same size on screen than
// it really has (1 pixel = 1 texel), i.e. DEBUG_RECT_FACTOR is then ignored.
void displayTextureRect(GLuint id_texture, unsigned pos_x, unsigned pos_y,
						unsigned tex_width, unsigned tex_height, bool original_size)
{
	Disable<GL_DEPTH_TEST> depth_test_state;

	// Bind the VAO:
	glBindVertexArray(id_vao);

	GLfloat fw = GLfloat(tex_width);
	GLfloat fh = GLfloat(tex_height);

	// Texture:
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, id_texture);

	// Texture parameters:
	TexParameter<GL_TEXTURE_RECTANGLE, GL_TEXTURE_COMPARE_MODE, GL_NONE> tex_compare_mode;

	// Setup shader and its uniforms:
	assert(debug_texture_rect_program != NULL);
	debug_texture_rect_program->use();
	debug_texture_rect_program->sendUniform("texunit", 0);

	// Compute position and size:
	GLfloat x = 0.0f;
	GLfloat y = 0.0f;
	GLfloat w = 0.0f;
	GLfloat h = 0.0f;

	// Case original_size == false (default): use DEBUG_RECT_FACTOR, etc.
	if(original_size == false)
	{
		// Get the window's aspect:
		GLfloat win_aspect = 0.0f;
		{
			int win_w=0, win_h=0;
			glfwGetWindowSize(&win_w, &win_h);
			win_h = (win_h <= 0 ? 1 : win_h);
			win_aspect = GLfloat(win_w) / GLfloat(win_h);
		}

		GLfloat y_aspect_factor = win_aspect * fh / fw;

		float reduc_factor = 1.0f / float(DEBUG_RECT_FACTOR);
		x = 2.0f * (reduc_factor * GLfloat(pos_x)                  ) - 1.0f;
		y = 2.0f * (reduc_factor * GLfloat(pos_y) * y_aspect_factor) - 1.0f;
		w = reduc_factor * 2.0f;
		h = w                  * y_aspect_factor;
	}
	// Case original_size == true: the resulting rectangle is the size it has in reality:
	else
	{
		// Get the window's size:
		int win_w=0, win_h=0;
		glfwGetWindowSize(&win_w, &win_h);
		win_h = (win_h <= 0 ? 1 : win_h);

		w = 2.0f * tex_width  / GLfloat(win_w);
		h = 2.0f * tex_height / GLfloat(win_h);
		x = GLfloat(pos_x) * w - 1.0f;
		y = GLfloat(pos_y) * h - 1.0f;
	}

	// Draw :
	GLfloat vertices[] = {x  , y,
						  x+w, y,
						  x+w, y+h,

						  x,   y,
						  x+w, y+h,
						  x,   y+h};
	const GLsizei nb_vertices = sizeof(vertices) / (2*sizeof(float));

	GLfloat texcoords[nb_vertices*2] = {	0.0f, 0.0f,
											fw,   0.0f,
											fw,   fh,

											0.0f, 0.0f,
											fw,   fh,
											0.0f, fh   };

	// - bind the VBO
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_quad);

	// - enable the vertex attributes arrays
	glEnableVertexAttribArray(DEBUG_TEXTURE_ATTRIB_POSITION);
	glEnableVertexAttribArray(DEBUG_TEXTURE_ATTRIB_TEXCOORDS);

	// - put data in the VBO and set the vertex attrib offsets:
	GLintptr offset = 0;
	GLsizeiptr size = 0;

	// - => vertices:
	offset = 0;
	size = sizeof(GLfloat)*nb_vertices*2;
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, vertices);
	glVertexAttribPointer(DEBUG_TEXTURE_ATTRIB_POSITION,  2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);

	// - => texcoords:
	offset += size;
	size = sizeof(GLfloat)*nb_vertices*2;
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, texcoords);
	glVertexAttribPointer(DEBUG_TEXTURE_ATTRIB_TEXCOORDS, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)offset);

	// - draw
	glDrawArrays(GL_TRIANGLES, 0, nb_vertices);
}

// Draw a point of the specified color
void drawPoint(const vec3& position,
			   const mat4& proj_matrix,
			   const mat4& view_matrix,
			   const vec3& color,
			   GLfloat point_size)
{
	Enable<GL_DEPTH_TEST> depth_test_state;

	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Point size:
	glPointSize(point_size);

	// Use the shader:
	assert(debug_unicolor_program != NULL);
	debug_unicolor_program->use();

	// Send the uniform matrices:
	debug_unicolor_program->sendUniform("proj_matrix", proj_matrix, false);
	debug_unicolor_program->sendUniform("view_matrix", view_matrix, false);

	// Bind and fill the VBO:
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_point);
	glEnableVertexAttribArray(DEBUG_UNICOLOR_ATTRIB_POSITION);
	glEnableVertexAttribArray(DEBUG_UNICOLOR_ATTRIB_COLOR);

	// x, y, z, r, g, b
	GLfloat values[] = {position.x, position.y, position.z, color.r, color.g, color.b};

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(values), values);

	// Set the vertex attrib pointers
	glVertexAttribPointer(DEBUG_UNICOLOR_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(0*sizeof(GLfloat)));
	glVertexAttribPointer(DEBUG_UNICOLOR_ATTRIB_COLOR,    3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(3*sizeof(GLfloat)));

	// Draw !
	glDrawArrays(GL_POINTS, 0, 1);

	// Cleanup:
	glPointSize(1.0f);
}

// Draw a line of the specified color
void drawLine(const vec3& v1,
			  const vec3& v2,
			  const mat4& proj_matrix,
			  const mat4& view_matrix,
			  const vec3& color,
			  GLfloat point_size)
{
	// Bind the VAO:
	glBindVertexArray(id_vao);

	// Point size:
	glPointSize(point_size);

	// Use the shader:
	assert(debug_unicolor_program != NULL);
	debug_unicolor_program->use();

	// Send the uniform matrices:
	debug_unicolor_program->sendUniform("proj_matrix", proj_matrix, false);
	debug_unicolor_program->sendUniform("view_matrix", view_matrix, false);

	// Bind and fill the VBO:
	glBindBuffer(GL_ARRAY_BUFFER, id_vbo_line);
	glEnableVertexAttribArray(DEBUG_UNICOLOR_ATTRIB_POSITION);
	glEnableVertexAttribArray(DEBUG_UNICOLOR_ATTRIB_COLOR);

	GLfloat values[] = {	v1.x, v1.y, v1.z, v2.x, v2.y, v2.z,
							color.r, color.g, color.b, color.r, color.g, color.b};

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(values), values);

	// Set the vertex attrib pointers
	glVertexAttribPointer(DEBUG_UNICOLOR_ATTRIB_POSITION, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(0*sizeof(GLfloat)));
	glVertexAttribPointer(DEBUG_UNICOLOR_ATTRIB_COLOR,    3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(6*sizeof(GLfloat)));

	// Draw !
	glDrawArrays(GL_LINES, 0, 2);

	// Cleanup:
	glPointSize(1.0f);
}

}	// namespace glutil
