// XMLManip.cpp

#include "XMLManip.h"
#include "../tinyxml/tinyxml.h"
#include <assert.h>

void readXMLColor(vec3* pvec, const TiXmlElement* element)
{
	double r=0.0, g=0.0, b=0.0;
	element->Attribute("r", &r);
	element->Attribute("g", &g);
	element->Attribute("b", &b);

	pvec->r = r;
	pvec->g = g;
	pvec->b = b;
}

void readXMLColor(vec4* pvec, const TiXmlElement* element)
{
	double r=0.0, g=0.0, b=0.0, a=1.0;
	element->Attribute("r", &r);
	element->Attribute("g", &g);
	element->Attribute("b", &b);
	element->Attribute("a", &a);

	pvec->r = r;
	pvec->g = g;
	pvec->b = b;
	pvec->a = a;
}

void readXMLValue(float* pf,  const TiXmlElement* element)
{
	double d=0.0;
	element->Attribute("value", &d);
	*pf = d;
}

void readXMLValue(uint* pi,  const TiXmlElement* element)
{
	int i=0;
	element->Attribute("value", &i);
	assert(i >= 0);
	*pi = (uint)i;
}
