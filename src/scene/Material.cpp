// Material.cpp

#include "Material.h"
#include "profiles/Profile.h"
#include "../Common.h"
#include "../log/Log.h"
#include "../tinyxml/tinyxml.h"
#include "../utils/XMLManip.h"
#include "../utils/StdListManip.h"
#include "../utils/StrManip.h"
#include "../utils/TGALoader.h"
#include <list>
using namespace std;

// ---------------------------------------------------------------------
Material::Material()
: flags(0)
{
	for(uint i=0 ; i < NB_MAX_MATERIAL_PROFILES ; i++)
		profiles[i] = NULL;
}

Material::~Material()
{
	for(uint i=0 ; i < NB_MAX_MATERIAL_PROFILES ; i++)
	{
		if(profiles[i] != NULL)
		{
			profiles[i]->unload();
			delete profiles[i];
			profiles[i] = NULL;
		}
	}
}

// ---------------------------------------------------------------------
Profile* Material::getProfile(uint index)
{
	assert(index <= NB_MAX_MATERIAL_PROFILES);
	return profiles[index];
}

const Profile* Material::getProfile(uint index) const
{
	assert(index <= NB_MAX_MATERIAL_PROFILES);
	return profiles[index];
}

// ---------------------------------------------------------------------
bool Material::loadFromXML(const string& filename)
{
	logInfo("loading material \"", filename, "\"");

	TiXmlDocument doc;

	// Load the file and check if it is valid
	this->filename = filename;
	if(!doc.LoadFile(filename))
	{
		logFailed("unable to load the requested file \"", filename, "\"");
		return false;
	}

	// Read/check the <material> mark:
	TiXmlElement* material_element = doc.FirstChildElement("material");

	if(!material_element)
	{
		logFailed("no <material> markup found in \"", filename, "\"");
		return false;
	}

	// Read the flags:
	for(TiXmlElement* flag_element = material_element->FirstChildElement("flag") ;
		flag_element != NULL ;
		flag_element = flag_element->NextSiblingElement("flag"))
	{
		const char* flag_value = flag_element->Attribute("value");
		if(!flag_value)
		{
			logFailed("<flag> markup without value attribute found in \"", filename, "\"");
			return false;
		}

		const std::string str_flag_value = flag_value;
		if(str_flag_value == MATERIAL_OPAQUE_STR)
			addFlags(MATERIAL_OPAQUE);
		else if(str_flag_value == MATERIAL_TRANSPARENT_STR)
			addFlags(MATERIAL_TRANSPARENT);
		else
		{
			logFailed("flag value \"", str_flag_value, "\" unknown");
			return false;
		}
	}

	// If neither opaque nor transparent is defined, add it manually and emit a warning:
	if(!(flags & MATERIAL_OPAQUE) && !(flags & MATERIAL_TRANSPARENT))
	{
		logWarn("material \"", filename, "\" is neither marked as opaque nor transparent, marking it as opaque");
		addFlags(MATERIAL_OPAQUE);
	}

	// Check we do not have a material being marked opaque and transparent at the same time:
	if((flags & MATERIAL_OPAQUE) && (flags & MATERIAL_TRANSPARENT))
	{
		logWarn("material \"", filename, "\" is marked as opaque and transparent");
		assert(false);
	}

	// Replace the <copy_contents/> marks with actual content:
	if(!replaceCopyContentsMarks(material_element))
		return false;

	// Read the profiles and load them:
	for(TiXmlElement* profile_element = material_element->FirstChildElement("profile") ;
		profile_element != NULL ;
		profile_element = profile_element->NextSiblingElement("profile"))
	{
		std::string profile_name = safeString(profile_element->Attribute("name"));
		uint index = 0;

		Profile* new_profile = Profile::create(profile_name, &index);
		new_profile->loadFromXML(filename, profile_element);
		this->profiles[index] = new_profile;
	}

	logSuccess("material \"", filename, "\"");

	return true;
}

// ---------------------------------------------------------------------
// Replace the <copy_contents/> marks with actual content:
bool Material::replaceCopyContentsMarks(TiXmlElement* material_element)
{
	// For all <profile>:
	for(TiXmlElement* profile_element = material_element->FirstChildElement("profile") ;
		profile_element != NULL ;
		profile_element = profile_element->NextSiblingElement("profile"))
	{
		// For all <copy_contents>:
		for(TiXmlElement* copy_contents_element = profile_element->FirstChildElement("copy_contents") ;
			copy_contents_element != NULL ;)
		{
			// Read the name of the profile to copy:
			const char* copied_profile_name = copy_contents_element->Attribute("name");
			if(copied_profile_name == NULL)
			{
				logFailed("<copy_contents> without name attribute");
				return false;
			}

			// Find the profile to copy:
			TiXmlElement* copied_profile_element = material_element->FirstChildElement("profile");
			for(; copied_profile_element != NULL ;
				copied_profile_element = copied_profile_element->NextSiblingElement("profile"))
			{
				const std::string candidate_profile_name = safeString(copied_profile_element->Attribute("name"));
				if(candidate_profile_name == copied_profile_name)
					break;
			}

			if(copied_profile_element == NULL)
			{
				logFailed("<copy_contents> references profile named \"", copied_profile_name,
						  "\" but there is no such profile");
				return false;
			}

			// Copy the profile's contents:
			for(TiXmlNode* copied_node = copied_profile_element->FirstChild() ;
				copied_node != NULL ;
				copied_node = copied_node->NextSibling())
			{
				profile_element->InsertBeforeChild(copy_contents_element, *copied_node);
			}

			// Remove the <copy_contents> mark:
            TiXmlElement* next = copy_contents_element->NextSiblingElement("copy_contents");
			profile_element->RemoveChild(copy_contents_element);
            copy_contents_element = next;
		}
	}

	return true;
}

// ---------------------------------------------------------------------
bool Material::isSymbolDefined(const char* symbol_name, const Preprocessor::SymbolList& preproc_sym)
{
	assert(symbol_name != NULL);

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
