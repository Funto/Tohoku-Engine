// Element.h

#ifndef ELEMENT_H
#define ELEMENT_H

#include "../Common.h"
#include <string>

class Element
{
private:
	vec3 position;
	mat3 orientation;
	std::string name;

public:
	Element();
	virtual ~Element();

	void setPosition(const vec3& pos);
	const vec3& getPosition() const;

	void setOrientation(const mat3& orientation);
	const mat3& getOrientation() const;

	void setName(const std::string& name);
	const std::string& getName() const;
};

#endif // ELEMENT_H
