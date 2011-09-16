// GPUProfile.cpp

#include "GPUProfile.h"
#include "../../Boundaries.h"
#include "../../tinyxml/tinyxml.h"
#include "../../log/Log.h"
#include "../../utils/XMLManip.h"
#include "../../utils/StdListManip.h"
#include "../../utils/StrManip.h"
#include "../../utils/TGALoader.h"
using namespace std;

GPUProfile::GPUProfile()
: Profile(),
  textures_loaded(false),
  program_loaded(false),
  program(NULL),
  program_id(),
  original_program_id(),
  has_texture_mapping(false),
  has_normal_mapping(false),
  textures(NULL),           nb_textures(0),
  uniforms_float(NULL),     nb_uniforms_float(0),
  uniforms_vec3(NULL),      nb_uniforms_vec3(0),
  uniforms_vec4(NULL),      nb_uniforms_vec4(0),
  uniforms_sampler2D(NULL), nb_uniforms_sampler2D(0)
{
}

GPUProfile::~GPUProfile()
{
	unload();
}

// ---------------------------------------------------------------------
// Implement the "Profile" interface:
void GPUProfile::loadFromXML(const string& filename, const TiXmlElement *profile_element)
{
	// Set the flags at their default values:
	this->has_texture_mapping = false;
	this->has_normal_mapping  = false;

	// Get the "program" element
	const TiXmlElement* program_element;
	if((program_element = profile_element->FirstChildElement("program")) == NULL)
	{
		logError("in \"", filename, "\": no <program> markup in a GPU profile");
		return;
	}

	// Get the filenames for the program
	this->original_program_id.vertex_filename   = string("media/shaders/") +
												   safeString(program_element->Attribute("vertex"));
	this->original_program_id.fragment_filename = string("media/shaders/") +
												   safeString(program_element->Attribute("fragment"));

	// Get the symbols for the program
	for(const TiXmlElement* symbol_element = profile_element->FirstChildElement("symbol") ;
		symbol_element != NULL ;
		symbol_element = symbol_element->NextSiblingElement("symbol"))
	{
		string symbol_name = safeString(symbol_element->Attribute("name"));

		if(symbol_name == "TEXTURE_MAPPING")
			this->has_texture_mapping = true;
		if(symbol_name == "NORMAL_MAPPING")
			this->has_normal_mapping  = true;

		this->original_program_id.preproc_sym.push_back(PreprocSym(symbol_name.c_str()));
	}

	// Get the textures:
	list<Texture> textures;

	for(const TiXmlElement* texture_element = profile_element->FirstChildElement("texture") ;
		texture_element != NULL ;
		texture_element = texture_element->NextSiblingElement("texture"))
	{
		int unit=0;
		Texture t;
		t.name = safeString(texture_element->Attribute("name"));
		texture_element->Attribute("unit", &unit);
		t.unit = uint(unit);
		t.id = 0;	// texture not loaded by default

		textures.push_back(t);
	}
	this->textures = listToArray(textures);
	this->nb_textures = textures.size();

	// Get the uniforms:
	// - float:
	list<UniformFloat> uniforms_float;

	for(const TiXmlElement* float_element = profile_element->FirstChildElement("float") ;
		float_element != NULL ;
		float_element = float_element->NextSiblingElement("float"))
	{
		UniformFloat u;
		u.name = safeString(float_element->Attribute("name"));
		double d=0.0;
		float_element->Attribute("value", &d);
		u.value = float(d);

		uniforms_float.push_back(u);
	}
	this->uniforms_float = listToArray(uniforms_float);
	this->nb_uniforms_float = uniforms_float.size();

	// - vec3:
	list<UniformVec3> uniforms_vec3;

	for(const TiXmlElement* vec3_element = profile_element->FirstChildElement("vec3") ;
		vec3_element != NULL ;
		vec3_element = vec3_element->NextSiblingElement("vec3"))
	{
		UniformVec3 u;
		u.name = safeString(vec3_element->Attribute("name"));
		readXMLColor(&u.value, vec3_element);

		uniforms_vec3.push_back(u);
	}
	this->uniforms_vec3 = listToArray(uniforms_vec3);
	this->nb_uniforms_vec3 = uniforms_vec3.size();

	// - vec4:
	list<UniformVec4> uniforms_vec4;

	for(const TiXmlElement* vec4_element = profile_element->FirstChildElement("vec4") ;
		vec4_element != NULL ;
		vec4_element = vec4_element->NextSiblingElement("vec4"))
	{
		UniformVec4 u;
		u.name = safeString(vec4_element->Attribute("name"));
		readXMLColor(&u.value, vec4_element);

		uniforms_vec4.push_back(u);
	}
	this->uniforms_vec4 = listToArray(uniforms_vec4);
	this->nb_uniforms_vec4 = uniforms_vec4.size();

	// - sampler2D:
	list<UniformSampler2D> uniforms_sampler2D;

	for(const TiXmlElement* sampler2D_element = profile_element->FirstChildElement("sampler2D") ;
		sampler2D_element != NULL ;
		sampler2D_element = sampler2D_element->NextSiblingElement("sampler2D"))
	{
		UniformSampler2D u;
		u.name = safeString(sampler2D_element->Attribute("name"));
		int value=0;
		sampler2D_element->Attribute("value", &value);
		u.value = uint(value);

		uniforms_sampler2D.push_back(u);
	}
	this->uniforms_sampler2D = listToArray(uniforms_sampler2D);
	this->nb_uniforms_sampler2D = uniforms_sampler2D.size();
}

void GPUProfile::unload()
{
	if(program_loaded)
		unloadProgram();

	if(textures_loaded)
		unloadTextures();

	delete [] textures;           nb_textures = 0;           textures = NULL;
	delete [] uniforms_float;     nb_uniforms_float = 0;     uniforms_float = NULL;
	delete [] uniforms_vec3;      nb_uniforms_vec3 = 0;      uniforms_vec3 = NULL;
	delete [] uniforms_vec4;      nb_uniforms_vec4 = 0;      uniforms_vec4 = NULL;
	delete [] uniforms_sampler2D; nb_uniforms_sampler2D = 0; uniforms_sampler2D = NULL;
}

// End of implementation of the Profile interface
// ---------------------------------------------------------------------

// For debugging:
void GPUProfile::assertTexunitsUnused(const uint* texunits, uint nb_texunits)
{
	for(uint i=0 ; i < nb_texunits ; i++)
	{
		uint texunit = texunits[i];

		for(uint j=0 ; j < nb_textures ; j++)
		{
			assert(texunit != textures[j].unit && ("texunit already in use!" || texunit));
		}
	}
}

// ---------------------------------------------------------------------
// Load/unload to/from GPU
// - textures:
void GPUProfile::loadTextures()
{
	assert(!this->textures_loaded);

	TGALoader tga;

	glActiveTexture(GL_TEXTURE0);
	for(uint i=0 ; i < this->nb_textures ; i++)
	{
		Texture& texture = this->textures[i];
		string path = string("media/textures/") + texture.name;

		if(tga.loadFile(path.c_str()) == TGA_OK)
		{
			glGenTextures(1, &texture.id);
			glBindTexture(GL_TEXTURE_2D, texture.id);

			GLuint internal_format = GL_RGBA;
			GLuint format = GL_RGBA;

			if(tga.getBpp() == 3)
			{
				logWarn("image \"", texture.name, "\" doesn't have an alpha channel");
				format = GL_RGB;
				internal_format = GL_RGB;
			}
			else if(tga.getBpp() != 4)
			{
				assert(false);
			}

			// Send the data:
			glTexImage2D(GL_TEXTURE_2D,
						 0,	// level
						 internal_format,	// internal format
						 tga.getWidth(), tga.getHeight(),
						 0,	// border
						 format,
						 GL_UNSIGNED_BYTE,
						 tga.getData());

			// Set the filter:
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			// Generate the mipmaps:
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}

	this->textures_loaded = true;
}

void GPUProfile::unloadTextures()
{
	assert(this->textures_loaded);

	// Delete the OpenGL textures
	for(uint i=0 ; i < this->nb_textures ; i++)
	{
		Texture& texture = this->textures[i];
		glDeleteTextures(1, &texture.id);
		texture.id = 0;
	}

	this->textures_loaded = false;
}

// - program:
void GPUProfile::loadProgram()
{
	Preprocessor::SymbolList additional_preproc_sym;	// empty list
	loadProgram(additional_preproc_sym);
}

void GPUProfile::loadProgram(const Preprocessor::SymbolList& additional_preproc_sym)
{
	assert(!this->program_loaded);

	// Setup a new GPUProgramID identifying the program.
	// It is equal to the original program ID (described in the XML file) + the additional preprocessor definitions.
	program_id = this->original_program_id;
	vectCat(&program_id.preproc_sym, &additional_preproc_sym);

	// Get or create the GPUProgram
	bool is_new = false;
	this->program = getProgramManager().getProgram(program_id, &is_new);

	if(is_new)
		onNewProgram(this->program.ptr());

	this->program_loaded = true;
}

void GPUProfile::unloadProgram()
{
	assert(this->program_loaded);
	this->program = NULL;	// decrement the reference count
	this->program_loaded = false;
}

// Bind for rendering:
void GPUProfile::bind()
{
	// Bind the textures:
	for(uint i = 0 ; i < nb_textures ; i++)
	{
		Texture& texture = textures[i];
		glActiveTexture(GL_TEXTURE0 + texture.unit);
		glBindTexture(GL_TEXTURE_2D, texture.id);
	}

	// Bind the uniforms:
	// - floats:
	for(uint i = 0 ; i < nb_uniforms_float ; i++)
		program->sendUniform(uniforms_float[i].name.c_str(), uniforms_float[i].value, Hash::AT_RUNTIME);

	// - vec3:
	for(uint i = 0 ; i < nb_uniforms_vec3 ; i++)
		program->sendUniform(uniforms_vec3[i].name.c_str(), uniforms_vec3[i].value, Hash::AT_RUNTIME);

	// - vec4:
	for(uint i = 0 ; i < nb_uniforms_vec4 ; i++)
		program->sendUniform(uniforms_vec4[i].name.c_str(), uniforms_vec4[i].value, Hash::AT_RUNTIME);

	// - sampler2D:
	for(uint i = 0 ; i < nb_uniforms_sampler2D ; i++)
		program->sendUniform(uniforms_sampler2D[i].name.c_str(), GLint(uniforms_sampler2D[i].value), Hash::AT_RUNTIME);
}

// Helper function to determine if a symbol is defined (i.e. its name is present somewhere in a
// given list of symbols)
bool GPUProfile::isSymbolDefined(const char* symbol_name, const GPUProgramID& prog_id)
{
	assert(symbol_name != NULL);

	const Preprocessor::SymbolList& preproc_sym = prog_id.preproc_sym;
	Preprocessor::SymbolList::const_iterator it     = preproc_sym.begin();
	Preprocessor::SymbolList::const_iterator it_end = preproc_sym.end();

	for(; it != it_end ; it++)
	{
		const PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return true;
	}

	return false;
}
