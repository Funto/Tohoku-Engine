// RaytraceProfile.cpp

#include "RaytraceProfile.h"
#include "../../tinyxml/tinyxml.h"
#include "../../utils/XMLManip.h"

RaytraceProfile::RaytraceProfile()
: diffuse(0.0), emissive(0.0), reflection(0.0f)
{
}

RaytraceProfile::~RaytraceProfile()
{
}

void RaytraceProfile::loadFromXML(const std::string& filename, const TiXmlElement *profile_element)
{
	const TiXmlElement* value_element;

	// - diffuse :
	if((value_element = profile_element->FirstChildElement("diffuse")) != NULL)
		readXMLColor(&diffuse, value_element);

	// - ambient :
	if((value_element = profile_element->FirstChildElement("emissive")) != NULL)
		readXMLColor(&emissive, value_element);

	// - reflection :
	if((value_element = profile_element->FirstChildElement("reflection")) != NULL)
		readXMLValue(&reflection, value_element);
}

void RaytraceProfile::unload()
{
	diffuse = vec3(0.0);
	emissive = vec3(0.0);
	reflection = 0.0f;
}
