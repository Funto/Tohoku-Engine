// Profile.h
// Interface describing a material profile

#ifndef PROFILE_H
#define PROFILE_H

#include "../../Common.h"
#include <string>
class TiXmlElement;

class Profile
{
public:
	enum Type
	{
		PROFILE_GENERAL,
		PROFILE_RAYTRACE,
		PROFILE_DEPTH_PEELING
	};

public:
	Profile() {}
	virtual ~Profile() {}

	virtual void loadFromXML(const std::string& filename, const TiXmlElement* profile_element) = 0;
	virtual void unload() = 0;

	virtual Type getType() const = 0;

	// Factory for creating a profile
	static Profile* create(const std::string& name, uint* pindex=NULL);
};

#endif // PROFILE_H
