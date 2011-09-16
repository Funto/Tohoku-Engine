// RaytraceProfile.h

#ifndef RAYTRACE_PROFILE_H
#define RAYTRACE_PROFILE_H

#include "Profile.h"
#include "../../Common.h"

class RaytraceProfile : public Profile
{
private:
	vec3 diffuse;
	vec3 emissive;
	float reflection;

public:
	RaytraceProfile();
	virtual ~RaytraceProfile();

	virtual void loadFromXML(const std::string& filename, const TiXmlElement *profile_element);
	virtual void unload();

	virtual Type getType() const {return PROFILE_RAYTRACE;}

	const vec3& getDiffuse()    const {return diffuse;}
	const vec3& getEmissive()   const {return diffuse;}
	float       getReflection() const {return reflection;}
};

#endif // RAYTRACE_PROFILE_HH
