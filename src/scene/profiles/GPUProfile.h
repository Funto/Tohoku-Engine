// GPUProfile.h

#ifndef GPU_PROFILE_H
#define GPU_PROFILE_H

#include "Profile.h"
#include "../../Common.h"
#include "../../glutil/glxw.h"
#include "../GPUProgramManager.h"

class GPUProfile : public Profile
{
public:
	// NB: Texture::unit ranges from 0 to 31 (OpenGL defines GL_TEXTURE0 through GL_TEXTURE31).
	struct Texture			{std::string name; GLuint id; uint unit;};
	struct UniformFloat		{std::string name; float value;};
	struct UniformVec3		{std::string name; vec3 value;};
	struct UniformVec4		{std::string name; vec4 value;};
	struct UniformSampler2D	{std::string name; uint value;};

protected:
	// Are the textures / program on the GPU?
	bool textures_loaded;
	bool program_loaded;

	// A program is being preprocessed using:
	// - normal preprocessor definitions (given by the Material's XML file, recorded in original_program_id)
	// - additional preprocessor definitions defined by the renderer (when calling loadProgram()).
	GPUProgramRef program;

	// vertex/fragment filenames + normal preprocessor definitions
	//                           + additional preprocessor definitions
	// This really identifies the program.
	GPUProgramID program_id;

	// vertex/fragment filenames + normal preprocessor definitions
	// This corresponds to what is written in the XML file.
	GPUProgramID original_program_id;

	// Flags which we could deduce from original_program_id.preproc_sym
	// This duplication is for optimization only.
	bool has_texture_mapping;
	bool has_normal_mapping;

	Texture*          textures;             uint nb_textures;
	UniformFloat*     uniforms_float;       uint nb_uniforms_float;
	UniformVec3*      uniforms_vec3;        uint nb_uniforms_vec3;
	UniformVec4*      uniforms_vec4;        uint nb_uniforms_vec4;
	UniformSampler2D* uniforms_sampler2D;   uint nb_uniforms_sampler2D;

public:
	GPUProfile();
	virtual ~GPUProfile();

	// Implement the "Profile" interface:
	virtual void loadFromXML(const std::string& filename, const TiXmlElement* profile_element);
	virtual void unload();

	// For debugging:
	void assertTexunitsUnused(const uint* texunits, uint nb_texunits);

	// Load/unload to/from GPU.
	// - textures:
	virtual void loadTextures();
	virtual void unloadTextures();
	inline bool areTexturesLoaded() const {return textures_loaded;}

	// - program:
	virtual void loadProgram();
	virtual void loadProgram(const Preprocessor::SymbolList& additional_preproc_sym);
	virtual void unloadProgram();
	inline bool isProgramLoaded() const {return program_loaded;}

	// Bind for rendering:
	virtual void bind();

	// Getters
	bool hasTextureMapping() const {return has_texture_mapping;}
	bool hasNormalMapping() const {return has_normal_mapping;}

	glutil::GPUProgram* getProgram() {return program.ptr();}
	const glutil::GPUProgram* getProgram() const {return program.ptr();}

	const Texture* getTextures() const {return textures;}
	uint getNbTextures() const {return nb_textures;}


protected:
	// Callback function which has to be defined in the children classes.
	// It is called when a new program is created, and is supposed to bind vertex attributes,
	// set uniform names and stuff like that.
	virtual void onNewProgram(glutil::GPUProgram* program) = 0;

	// Helper function to determine if a symbol is defined (i.e. its name is present somewhere in a
	// given list of symbols)
	static bool isSymbolDefined(const char* symbol_name, const GPUProgramID& prog_id);
};

#endif // GPU_PROFILE_H
