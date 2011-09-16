// XMLManip.h

#ifndef XML_MANIP_H
#define XML_MANIP_H

#include "../Common.h"

class TiXmlElement;

// Functions for reading values from XML files,
// example : <diffuse r="1.0" g="1.0" b="1.0" a="1.0"/>
void readXMLColor(vec3* pvec, const TiXmlElement* element);
void readXMLColor(vec4* pvec, const TiXmlElement* element);
void readXMLValue(float* pf,  const TiXmlElement* element);
void readXMLValue(uint* pi,  const TiXmlElement* element);

#endif // XML_MANIP_H
