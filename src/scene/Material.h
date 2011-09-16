// Material.h

#ifndef MATERIAL_H
#define MATERIAL_H

#include "../Common.h"
#include "../Boundaries.h"
#include "../scene/GPUProgramManager.h"
#include <string>

class TiXmlElement;
class Profile;

// Flags for the material:
#define MATERIAL_OPAQUE      (1 << 0)
#define MATERIAL_TRANSPARENT (1 << 1)

// Values to be found in the XML file
// (ex: <flag value="opaque"/>)
#define MATERIAL_OPAQUE_STR      "opaque"
#define MATERIAL_TRANSPARENT_STR "transparent"

class Material
{
public:
	typedef unsigned int Flags;

private:
	std::string filename;
	Profile* profiles[NB_MAX_MATERIAL_PROFILES];
	Flags flags;

public:
	Material();
	virtual ~Material();

	const std::string& getFilename() const {return filename;}

	Profile* getProfile(uint index);
	const Profile* getProfile(uint index) const;

	// Load the material's profiles from an XML file
	bool loadFromXML(const std::string& filename);

	// Flags:
	void setFlags(Flags flags) {this->flags = flags;}
	void addFlags(Flags flags) {this->flags |= flags;}
	Flags getFlags() const {return flags;}

private:
	static bool replaceCopyContentsMarks(TiXmlElement* material_element);
	static bool isSymbolDefined(const char* symbol_name, const Preprocessor::SymbolList& preproc_sym);
};

#endif // MATERIAL_H
