// GPUProgram.cpp

#include "GPUProgram.h"
#include "../log/Log.h"
#include <cstdarg>
#include <cassert>
#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
using namespace std;

// Define this to print preprocessed source
//#define DEBUG_PRINT_PREPROCESSED_SRC

#define UNKNOWN_UNIFORM -2	// This is the value returned by getUniformLocation()
							// when the uniform asked is not found, i.e. when
							// the user has not mentionned it in setUniformNames()

namespace glutil
{

GPUProgram::GPUProgram(const char* vertex_filename, const char* fragment_filename, bool debug_print)
: ref_count(0),
  id_program(0), id_vertex(0), id_fragment(0),
  debug_print(debug_print), compiled(false), linked(false),
  uniforms(NULL), nb_uniforms(0),
  preproc(NULL),
  vertex_filename(vertex_filename),
  fragment_filename(fragment_filename)
{
	// Create shaders and program:
	id_vertex = glCreateShader(GL_VERTEX_SHADER);		assert(id_vertex != 0);
	id_fragment = glCreateShader(GL_FRAGMENT_SHADER);	assert(id_fragment != 0);
	id_program = glCreateProgram();						assert(id_program != 0);

	// Create preprocessor:
	preproc = new Preprocessor(true);
}

GPUProgram::~GPUProgram()
{
	delete [] uniforms;

	delete preproc;

	glDeleteProgram(id_program);
	glDeleteShader(id_vertex);
	glDeleteShader(id_fragment);
}

// ---------------------------------------------------------------------
GLuint GPUProgram::getProgram() const
{
	return id_program;
}

GLuint GPUProgram::getVertexShader() const
{
	return id_vertex;
}

GLuint GPUProgram::getFragmentShader() const
{
	return id_fragment;
}

// ---------------------------------------------------------------------
const std::string& GPUProgram::getVertexFilename() const
{
	return vertex_filename;
}

const std::string& GPUProgram::getFragmentFilename() const
{
	return fragment_filename;
}

// ---------------------------------------------------------------------
Preprocessor* GPUProgram::getPreprocessor()
{
	return this->preproc;
}

const Preprocessor* GPUProgram::getPreprocessor() const
{
	return this->preproc;
}

// ---------------------------------------------------------------------
bool GPUProgram::compileAndAttach()
{
	// Load shader sources, preprocess them and send them to the GPU:
	// - vertex shader:
	if(!loadShaderSource(vertex_filename.c_str(), id_vertex))
	{
		compiled = false;
		assert(false);
		return compiled;
	}

	// - fragment shader:
	if(!loadShaderSource(fragment_filename.c_str(), id_fragment))
	{
		compiled = false;
		assert(false);
		return compiled;
	}

	// Compile shaders:
	GLint status;

	// - vertex shader:
	glCompileShader(id_vertex);
	if(debug_print)
		printShaderLog(id_vertex, vertex_filename, "vertex");
	glGetShaderiv(id_vertex, GL_COMPILE_STATUS, &status);
	compiled = (status == GL_TRUE);
	if(!compiled)
	{
		if(debug_print)
			logError("compiling vertex shader \"", vertex_filename, "\"");
		return false;
	}

	// - fragment shader:
	glCompileShader(id_fragment);
	if(debug_print)
		printShaderLog(id_fragment, fragment_filename, "fragment");
	glGetShaderiv(id_fragment, GL_COMPILE_STATUS, &status);
	compiled = (status == GL_TRUE);
	if(!compiled)
	{
		if(debug_print)
			logError("compiling fragment shader \"", fragment_filename, "\"");
		return false;
	}

	// Attach shaders:
	glAttachShader(id_program, id_vertex);
	glAttachShader(id_program, id_fragment);

	return compiled;	// if we reach this, compiled == true
}

// ---------------------------------------------------------------------
bool GPUProgram::isCompiled() const
{
	return compiled;
}

// ---------------------------------------------------------------------
void GPUProgram::bindAttribLocation(GLint location, const char* attrib_name)
{
	assert(id_program != 0);
	glBindAttribLocation(id_program, location, attrib_name);
}

void GPUProgram::bindAttribLocations(GLint location, const char* attrib_name, ...)
{
	va_list vl;
	va_start(vl, attrib_name);

	GLint loc = location;
	const char* name = attrib_name;

	while(name != NULL)
	{
		bindAttribLocation(loc, name);

		loc = va_arg(vl, GLint);
		name = va_arg(vl, const char*);
	}
	va_end(vl);
}

void GPUProgram::bindAttribLocations(const GPUProgram::Location* locations, uint nb_locations)
{
	for(uint i=0 ; i < nb_locations ; i++)
		bindAttribLocation(locations[i].location, locations[i].name.c_str());
}

// ---------------------------------------------------------------------
void GPUProgram::bindFragDataLocation(GLint location, const char* frag_data_name)
{
	assert(id_program != 0);
	glBindFragDataLocation(id_program, location, frag_data_name);
}

void GPUProgram::bindFragDataLocations(GLint location, const char* frag_data_name, ...)
{
	va_list vl;
	va_start(vl, frag_data_name);

	GLuint loc = location;
	const char* name = frag_data_name;

	while(name != NULL)
	{
		bindFragDataLocation(loc, name);

		loc = va_arg(vl, GLint);
		name = va_arg(vl, const char*);
	}
	va_end(vl);
}

void GPUProgram::bindFragDataLocations(const GPUProgram::Location* locations, uint nb_locations)
{
	for(uint i=0 ; i < nb_locations ; i++)
		bindFragDataLocation(locations[i].location, locations[i].name.c_str());
}

// ---------------------------------------------------------------------
void GPUProgram::setUniformNames(const char* uniform_name, ...)
{
	// Delete previous uniforms
	delete [] uniforms;
	nb_uniforms = 0;

	// Accumulate new symbols in a list and gather the uniform locations
	list<Uniform> uniform_list;
	const char* name = uniform_name;

	va_list vl;
	va_start(vl, uniform_name);
	while(name != NULL)
	{
		int val = Hash(name, Hash::AT_RUNTIME).val;
		GLint location = glGetUniformLocation(id_program, name);

		uniform_list.push_back(Uniform(val, location));

		name = va_arg(vl, const char*);
	}
	va_end(vl);

	// Copy the list to the member array
	nb_uniforms = uniform_list.size();
	uniforms = new Uniform[nb_uniforms];

	uint i=0;
	list<Uniform>::iterator it     = uniform_list.begin();
	list<Uniform>::iterator it_end = uniform_list.end();
	for( ;
		it != it_end ;
		it++, i++)
	{
		uniforms[i] = *it;
	}

	// Check if all hash values are different:
#ifndef NDEBUG
	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		for(uint j=i+1 ; j < nb_uniforms ; j++)
		{
			if(uniforms[i].hash_val == uniforms[j].hash_val)
			{
				logError("detected 2 hashes with same value in uniforms array !\n"
						 "value is: ", uniforms[i].hash_val, ", indices are ", i, " and ", j);
				assert(false);
			}
		}
	}
#endif
}

void GPUProgram::setUniformNames(const char** uniform_names, uint nb_uniforms)
{
	// Delete previous uniforms
	delete [] this->uniforms;

	// Create new uniforms and get their locations:
	this->uniforms = new Uniform[nb_uniforms];
	this->nb_uniforms = nb_uniforms;

	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		Uniform& u = this->uniforms[i];
		const char* name = uniform_names[i];

		u.hash_val = Hash(name, Hash::AT_RUNTIME).val;
		u.location = glGetUniformLocation(id_program, name);
	}

	// Check if all hash values are different:
#ifndef NDEBUG
	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		for(uint j=i+1 ; j < nb_uniforms ; j++)
		{
			if(uniforms[i].hash_val == uniforms[j].hash_val)
			{
				logError("detected 2 hashes with same value in uniforms array !\n"
						 "value is: ", uniforms[i].hash_val, ", indices are ", i, " and ", j);
				assert(false);
			}
		}
	}
#endif
}

void GPUProgram::setUniformNames(const string* uniform_names, uint nb_uniforms)
{
	// Delete previous uniforms
	delete [] this->uniforms;

	// Create new uniforms and get their locations:
	this->uniforms = new Uniform[nb_uniforms];
	this->nb_uniforms = nb_uniforms;

	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		Uniform& u = this->uniforms[i];
		const char* name = uniform_names[i].c_str();

		u.hash_val = Hash(name, Hash::AT_RUNTIME).val;
		u.location = glGetUniformLocation(id_program, name);
	}

	// Check if all hash values are different:
#ifndef NDEBUG
	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		for(uint j=i+1 ; j < nb_uniforms ; j++)
		{
			if(uniforms[i].hash_val == uniforms[j].hash_val)
			{
				logError("detected 2 hashes with same value in uniforms array !\n"
						 "value is: ", uniforms[i].hash_val, ", indices are ", i, " and ", j);
				assert(false);
			}
		}
	}
#endif
}

void GPUProgram::setUniformNames(const list<string>& uniform_names)
{
	// Delete previous uniforms
	delete [] uniforms;
	nb_uniforms = 0;

	// Copy the list to the member array while computing the hash values and
	// getting the uniform locations:
	nb_uniforms = uniform_names.size();
	uniforms = new Uniform[nb_uniforms];

	uint i=0;

	list<string>::const_iterator it     = uniform_names.begin();
	list<string>::const_iterator it_end = uniform_names.end();

	for( ;
		 it != it_end ;
		 it++, i++)
	{
		Uniform& uniform = uniforms[i];
		const char* name = it->c_str();

		uniform.hash_val = Hash(name, Hash::AT_RUNTIME).val;
		uniform.location = glGetUniformLocation(id_program, name);
	}

	// Check if all hash values are different:
#ifndef NDEBUG
	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		for(uint j=i+1 ; j < nb_uniforms ; j++)
		{
			if(uniforms[i].hash_val == uniforms[j].hash_val)
			{
				logError("detected 2 hashes with same value in uniforms array !\n"
						 "value is: ", uniforms[i].hash_val, ", indices are ", i, " and ", j);
				assert(false);
			}
		}
	}
#endif
}

// ---------------------------------------------------------------------
bool GPUProgram::link()
{
	// Link and print log
	assert(id_program != 0);
	glLinkProgram(id_program);
	if(debug_print)
		printProgramLog();

	// Check if the linkage was successful
	GLint flag = 0;
	glGetProgramiv(id_program, GL_LINK_STATUS, &flag);
	linked = (flag == GL_TRUE);
	return linked;
}

bool GPUProgram::isLinked() const
{
	return linked;
}

// ---------------------------------------------------------------------
bool GPUProgram::validate()
{
    return true;

	glValidateProgram(id_program);

	GLint status;
	glGetProgramiv(id_program, GL_VALIDATE_STATUS, &status);

	if(debug_print)
	{
		if(status == GL_TRUE)
			logSuccess("program validated");
		else
        {
			logFailed("program not validated");
            GLint max_len;
            GLint len;
            glGetProgramiv(id_program, GL_INFO_LOG_LENGTH, &max_len);
            char* info_log = new char[max_len+1];
            glGetProgramInfoLog(id_program, max_len, &len, info_log);

            logError("Shader info log: ", info_log);

            delete [] info_log;
        }
	}

	validated = (status == GL_TRUE);
	return validated;
}

bool GPUProgram::isValidated() const
{
	return validated;
}

// ---------------------------------------------------------------------
void GPUProgram::use()
{
	glUseProgram(id_program);
}

// ---------------------------------------------------------------------
void GPUProgram::sendUniform(Hash h, GLint i)
{
	glUniform1i(getUniformLocation(h), i);
}
void GPUProgram::sendUniform(Hash h, GLfloat f)
{
	glUniform1f(getUniformLocation(h), f);
}
void GPUProgram::sendUniform(Hash h, const vec3& v)
{
	glUniform3fv(getUniformLocation(h), 1, &v[0]);
}
void GPUProgram::sendUniform(Hash h, const vec4& v)
{
	glUniform4fv(getUniformLocation(h), 1, &v[0]);
}
void GPUProgram::sendUniform(Hash h, const mat3& m, bool transpose)
{
	glUniformMatrix3fv(getUniformLocation(h), 1, transpose, &m[0][0]);
}
void GPUProgram::sendUniform(Hash h, const mat4& m, bool transpose)
{
	glUniformMatrix4fv(getUniformLocation(h), 1, transpose, &m[0][0]);
}

// ---------------------------------------------------------------------
// Runtime versions:
#ifdef NDEBUG
#define CHECK_LOCATION(uniform_name, location)
#else
#define CHECK_LOCATION(uniform_name, location)	\
	if(location == UNKNOWN_UNIFORM) {	\
		logError("uniform \"", uniform_name, "\" is unknown !");	\
		assert(false);	\
	}
#endif


void GPUProgram::sendUniform(const char* uniform_name, GLint i,                       Hash::Marker marker)
{
	GLint location = getUniformLocation(Hash(uniform_name, Hash::AT_RUNTIME));
	CHECK_LOCATION(uniform_name, location);
	glUniform1i(location, i);
}

void GPUProgram::sendUniform(const char* uniform_name, GLfloat f,                     Hash::Marker marker)
{
	GLint location = getUniformLocation(Hash(uniform_name, Hash::AT_RUNTIME));
	CHECK_LOCATION(uniform_name, location);
	glUniform1f(location, f);
}

void GPUProgram::sendUniform(const char* uniform_name, const vec3& v,                 Hash::Marker marker)
{
	GLint location = getUniformLocation(Hash(uniform_name, Hash::AT_RUNTIME));
	CHECK_LOCATION(uniform_name, location);
	glUniform3fv(location, 1, &v[0]);
}

void GPUProgram::sendUniform(const char* uniform_name, const vec4& v,                 Hash::Marker marker)
{
	GLint location = getUniformLocation(Hash(uniform_name, Hash::AT_RUNTIME));
	CHECK_LOCATION(uniform_name, location);
	glUniform4fv(location, 1, &v[0]);
}

void GPUProgram::sendUniform(const char* uniform_name, const mat3& m, bool transpose, Hash::Marker marker)
{
	GLint location = getUniformLocation(Hash(uniform_name, Hash::AT_RUNTIME));
	CHECK_LOCATION(uniform_name, location);
	glUniformMatrix3fv(location, 1, transpose, &m[0][0]);
}

void GPUProgram::sendUniform(const char* uniform_name, const mat4& m, bool transpose, Hash::Marker marker)
{
	GLint location = getUniformLocation(Hash(uniform_name, Hash::AT_RUNTIME));
	CHECK_LOCATION(uniform_name, location);
	glUniformMatrix4fv(location, 1, transpose, &m[0][0]);
}

// ---------------------------------------------------------------------
// Load, preprocess and call glShaderSource()
bool GPUProgram::loadShaderSource(const char* filename, GLuint id_shader)
{
	// - load text from the file and preprocess it:
	const char* preprocessed = preproc->preprocessFromFile(filename);
	if(preprocessed == NULL)
		return false;

	// - debug: print the result of the preprocessing
#ifdef DEBUG_PRINT_PREPROCESSED_SRC
	cout << "\"" << filename << "\" (after preprocessing)" << endl;
	cout << "(symbol(s): " << preproc->getSymbolsString() << ")" << endl;
	cout << "===========================" << endl;
	cout << preprocessed << endl;
#endif

	// - set the shader's source on the GPU
	glShaderSource(id_shader, 1, &preprocessed, NULL);

	// - cleanup
	delete [] preprocessed;

	return true;
}

// ---------------------------------------------------------------------
inline GLint GPUProgram::getUniformLocation(Hash h) const
{
	for(uint i=0 ; i < nb_uniforms ; i++)
	{
		const Uniform& u = uniforms[i];
		if(u.hash_val == h.val)
			return u.location;
	}
	return UNKNOWN_UNIFORM;
}

// ---------------------------------------------------------------------
// Print the log of a shader compilation
void GPUProgram::printShaderLog(GLuint id_shader, const std::string& shader_name, const char* shader_type)
{
	GLchar* buf = NULL;
	GLint len = 0;

	// Get the log:
	glGetShaderiv(id_shader, GL_INFO_LOG_LENGTH, &len);

	buf = new GLchar[len];
	glGetShaderInfoLog(id_shader, len, &len, buf);

	// Print the log
	cout << "*** " << shader_type << " shader log: \"" << shader_name << "\" (original symbol(s): "
		 << preproc->getOriginalSymbolsString() << ")" << endl << buf;

	// Cleanup
	delete [] buf;
}

// ---------------------------------------------------------------------
// Print the log of linking a program
void GPUProgram::printProgramLog()
{
	GLchar* buf = NULL;
	GLint len = 0;

	// Get the log:
	glGetProgramiv(id_program, GL_INFO_LOG_LENGTH, &len);

	buf = new GLchar[len];
	glGetProgramInfoLog(id_program, len, &len, buf);

	// Print the log
	cout << "*** program log: \"" << vertex_filename << "\", \"" << fragment_filename << "\" (original symbol(s): "
		 << preproc->getOriginalSymbolsString() << ") " << endl << buf;

	// Cleanup
	delete [] buf;
}

}	// namespace glutil
