// glutil.h

#ifndef GL_UTIL_H
#define GL_UTIL_H

#include "glxw.h"
#include "RAII.h"
#include "Quad.h"
#include "GPUProgram.h"
#include "TextureCreation.h"

// Maximum number of OpenGL errors that are printed
// by checkGLErrors(). 0 for no limit.
#define MAX_GL_ERRORS 5

// Size of a debug quad. This is the size in pixels the quad would
// have in a resolution of 800x600. The quad is scaled if the resolution
// is different.
//#define DEBUG_QUAD_SIZE 128.0f
#define DEBUG_QUAD_SIZE 160.0f

// How many times is a rectangle of the same size (and aspect ratio)
// as the screen smaller that the screen when displayed with displayTextureRect().
#define DEBUG_RECT_FACTOR 5

#define GL_CHECK() glutil::checkGLErrors(__FILE__, __LINE__, __FUNCTION__)

// Bind a texture to a given texunit, send the corresponding uniform to the
// program and increment the texunit number.
#define BIND_TEX(target, id_texture, uniform_name, program, texunit)	\
	glActiveTexture(GL_TEXTURE0 + texunit);	\
	glBindTexture(target, id_texture);		\
	program->sendUniform(uniform_name, GLint(texunit));	\
	texunit++

namespace glutil
{
// Initialization / shutdown of the glutil library
void init();
void shutdown();

// Get a string describing an OpenGL error:
// Returns NULL if unknown error.
const char* getGLErrorString(GLenum error);

// Check OpenGL errors
bool checkGLErrors(const char* file="", unsigned line=0, const char* function="");

// Print information on the GPU memory
void printGPUMemoryInfo(const char* msg=NULL);

// Load a source file in a shader
bool loadShaderSource(GLuint id_shader, const char* file_path);

// Print the log of a shader compilation
void printShaderLog(GLuint id_shader, const char* shader_type);

// Print the log of linking a program
void printProgramLog(GLuint id_program);

// Check if a program is valid
bool checkProgramValid(GLuint id_program);

// Display a quad made of 2 triangles for debugging textures
// at position (pos_x, pos_y).
// Example: (0, 0) is bottom left, (1, 0) is the quad next to the right.
// The quads have a size of DEBUG_QUAD_SIZE by DEBUG_QUAD_SIZE.
void displayTexture2D(GLuint id_texture, unsigned pos_x, unsigned pos_y);

// Display a quad made of 2 triangles for debugging rectangle textures.
// (pos_x, pos_y) has the same meaning as for displayTexture2D().
// (tex_width, tex_height) is the size of the texture rectangle.
// If original_size == true, the quad will have the same size on screen than
// it really has (1 pixel = 1 texel), i.e. DEBUG_RECT_FACTOR is then ignored.
void displayTextureRect(GLuint id_texture, unsigned pos_x, unsigned pos_y,
						unsigned tex_width, unsigned tex_height, bool original_size=false);

// Draw a point of the specified color
void drawPoint(const vec3& position,
			   const mat4& proj_matrix,
			   const mat4& view_matrix,
			   const vec3& color=vec3(1.0, 1.0, 1.0),
			   GLfloat point_size=1.0f);

// Draw a line of the specified color
void drawLine(const vec3& v1,
			  const vec3& v2,
			  const mat4& proj_matrix,
			  const mat4& view_matrix,
			  const vec3& color=vec3(1.0, 1.0, 1.0),
			  GLfloat point_size=1.0f);

}

#endif // GL_UTIL_H
