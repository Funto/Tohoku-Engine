// GPUProgram.h

/* Example usage:
GPUProgram* p = new GPUProgram("shader.vert", "vertex.frag");
p->getPreprocessor()->setSymbols("DEFINED_SYMBOL_1", "DEFINED_SYMBOL_2", NULL);
bool ok = p->compileAndAttach();
assert(ok);
p->bindAttribLocations(ATTRIB_POSITION, "vertex_position", ATTRIB_NORMAL, "vertex_normal", 0, NULL);
p->bindFragDataLocations(FRAG_DATA_COLOR, "frag_color", 0, NULL);
ok &= p->link();
assert(ok);
p->setUniformNames("ambient_color", "texunit", NULL);
p->validate();
// -----
p->use();
p->sendUniform("texunit", 0);
// -----
delete p;
*/

#ifndef GPU_PROGRAM_H
#define GPU_PROGRAM_H

#include "../glutil/glxw.h"
#include "../Common.h"
#include "../preprocessor/Preprocessor.h"
#include "Hash.h"
#include <string>
#include <list>

namespace glutil
{

class GPUProgram
{
public:
	uint ref_count;	// used only when combined with GPUProgramRef and GPUProgramManager

	// Structure which can be used by the user of the class to specify vertex attribute
	// locations or fragment data locations.
	struct Location
	{
		GLint location;
		std::string name;

		Location(GLint location, const char* name)
		: location(location), name(name)
		{
		}
	};

private:
	// OpenGL objects
	GLuint id_program;
	GLuint id_vertex;
	GLuint id_fragment;

	bool debug_print;	// Should we print to the console ?
	bool compiled;	// Are both shaders compiled ?
	bool linked;	// Is the program linked ?
	bool validated;	// Is the program validated ?

	// Uniform variables locations:
	struct Uniform
	{
		int hash_val;
		GLint location;

		Uniform() {}
		Uniform(int hash_val, GLint location) : hash_val(hash_val), location(location) {}
	};

	Uniform* uniforms;
	uint nb_uniforms;

	// Preprocessor:
	Preprocessor* preproc;

	// Shaders filenames:
	std::string vertex_filename;
	std::string fragment_filename;

private:
	GPUProgram() {}
public:
	GPUProgram(const char* vertex_filename, const char* fragment_filename, bool debug_print=true);
	virtual ~GPUProgram();

	GLuint getProgram() const;
	GLuint getVertexShader() const;
	GLuint getFragmentShader() const;

	const std::string& getVertexFilename() const;
	const std::string& getFragmentFilename() const;

	Preprocessor* getPreprocessor();
	const Preprocessor* getPreprocessor() const;

	bool compileAndAttach();
	bool isCompiled() const;

	void bindAttribLocation(GLint location, const char* attrib_name);
	void bindAttribLocations(GLint location, const char* attrib_name, ...);
	void bindAttribLocations(const Location* locations, uint nb_locations);

	void bindFragDataLocation(GLint location, const char* frag_data_name);
	void bindFragDataLocations(GLint location, const char* frag_data_name, ...);
	void bindFragDataLocations(const Location* locations, uint nb_locations);

	void setUniformNames(const char* uniform_name, ...);
	void setUniformNames(const char** uniform_names, uint nb_uniforms);
	void setUniformNames(const std::string* uniform_names, uint nb_uniforms);
	void setUniformNames(const std::list<std::string>& uniform_names);

	bool link();
	bool isLinked() const;

	bool validate();
	bool isValidated() const;

	void use();

	// Usage example: sendUniform("texunit", value);
	void sendUniform(Hash h, GLint i);
	void sendUniform(Hash h, GLfloat f);
	void sendUniform(Hash h, const vec3& v);
	void sendUniform(Hash h, const vec4& v);
	void sendUniform(Hash h, const mat3& m, bool transpose=false);
	void sendUniform(Hash h, const mat4& m, bool transpose=false);

	// If needed at runtime: sendUniform(uniform_name, value, Hash::AT_RUNTIME)
	void sendUniform(const char* uniform_name, GLint i,                       Hash::Marker marker);
	void sendUniform(const char* uniform_name, GLfloat f,                     Hash::Marker marker);
	void sendUniform(const char* uniform_name, const vec3& v,                 Hash::Marker marker);
	void sendUniform(const char* uniform_name, const vec4& v,                 Hash::Marker marker);
	void sendUniform(const char* uniform_name, const mat3& m, bool transpose, Hash::Marker marker);
	void sendUniform(const char* uniform_name, const mat4& m, bool transpose, Hash::Marker marker);

private:
	// -------  Helper functions: ----------
	// Load, preprocess and call glShaderSource()
	bool loadShaderSource(const char* filename, GLuint id_shader);

	// Get a uniform location using the memorized "uniforms[]" array and a hash value
	inline GLint getUniformLocation(Hash h) const;

	// Print the log of a shader compilation
	void printShaderLog(GLuint id_shader, const std::string& shader_name, const char* shader_type);

	// Print the log of linking a program
	void printProgramLog();

	friend void gpuprogram_unittest();
};

}	// namespace glutil

#endif // GPU_PROGRAM_H
