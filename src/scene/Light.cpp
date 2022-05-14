// Light.cpp

#include "Light.h"
#include "LightData.h"
#include "../log/Log.h"
#include "../tinyxml/tinyxml.h"
#include "../utils/StrManip.h"
#include "../utils/XMLManip.h"
#include "../glutil/glutil.h"
#include "../glm/gtc/matrix_transform.hpp"
#include <cmath>
using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEFAULT_SHADOW_MAP_SIZE 512
#define DEFAULT_BOUNCE_MAP_SIZE 1024

#define LIGHT_ZNEAR 0.5f
#define LIGHT_ZFAR  12.0f

Light::Light()
: Element(),
  color(),
  angle(0.0f),
  shadow_map_size(DEFAULT_SHADOW_MAP_SIZE),
  bounce_map_size(DEFAULT_BOUNCE_MAP_SIZE)
{
	for(uint i=0 ; i < NB_MAX_LIGHT_DATA ; i++)
		this->user_data[i] = NULL;
}

Light::~Light()
{
	for(uint i=0 ; i < NB_MAX_LIGHT_DATA ; i++)
		setUserData(i, NULL);
}

// ---------------------------------------------------------------------
bool Light::loadFromXML(const string& filename)
{
	logInfo("loading light \"", filename, "\"");

	TiXmlDocument doc;

	// Load the file and check if it is valid
	if(!doc.LoadFile(filename))
	{
		logFailed("unable to load the requested file \"", filename, "\"");
		return false;
	}

	// Get the <light> element:
	TiXmlElement* light_element = doc.FirstChildElement("light");

	if(!light_element)
	{
		logFailed("no <light> mark found in \"", filename, "\"");
		return false;
	}

	// Read the values:
	const TiXmlElement* value_element = NULL;

	// - color :
	if((value_element = light_element->FirstChildElement("color")) != NULL)
		readXMLColor(&color, value_element);

	// - angle :
	if((value_element = light_element->FirstChildElement("angle")) != NULL)
		readXMLValue(&angle, value_element);

	// - shadow_map_size:
	if((value_element = light_element->FirstChildElement("shadow_map_size")) != NULL)
		readXMLValue(&shadow_map_size, value_element);

	// - bounce_map_size:
	if((value_element = light_element->FirstChildElement("bounce_map_size")) != NULL)
		readXMLValue(&bounce_map_size, value_element);

	logSuccess("light \"", filename, "\"");

	return true;
}

// View matrix
mat4 Light::computeViewMatrix() const
{
	mat4 m(glm::inverse(getOrientation()));
	return glm::translate(m, -getPosition());
}

// Projection matrix
mat4 Light::computeProjectionMatrix() const
{
	return glm::perspective(angle, 1.0f, LIGHT_ZNEAR, LIGHT_ZFAR);
}

// Debug drawing of the light:
void Light::debugDrawFrustum(const mat4& proj_matrix,
							 const mat4& view_matrix,
							 const vec3& color,
							 bool draw_far_plane)
{
	const vec3& pos = getPosition();
	const mat3& orientation = getOrientation();
	float angle_rad = M_PI*angle/180.0;

	float tan_half = tan(angle_rad/2.0);

	vec3 down_left  = vec3(-tan_half, -tan_half, -1.0);
	vec3 down_right = vec3( tan_half, -tan_half, -1.0);
	vec3 up_right   = vec3( tan_half,  tan_half, -1.0);
	vec3 up_left    = vec3(-tan_half,  tan_half, -1.0);

	// Positions of the 8 points in the corners of the frustum:
	vec3 corners[] = {
		LIGHT_ZNEAR * down_left, LIGHT_ZNEAR * down_right,
		LIGHT_ZNEAR * up_right,  LIGHT_ZNEAR * up_left,
		LIGHT_ZFAR  * down_left, LIGHT_ZFAR  * down_right,
		LIGHT_ZFAR  * up_right,  LIGHT_ZFAR  * up_left
	};

	// Move the points to the light space:
	for(uint i=0 ; i < 8 ; i++)
		corners[i] = pos + orientation*corners[i];

#define DRAW_LINE_CORNERS(i, j)   glutil::drawLine(corners[i], corners[j], proj_matrix, view_matrix, color, 1.0f)
#define DRAW_LINE_TO_CORNER(p, j) glutil::drawLine(p,          corners[j], proj_matrix, view_matrix, color, 1.0f)

	// Draw the lines linking the position of the light and the near plane:
	for(uint i=0 ; i < 4 ; i++)
		DRAW_LINE_TO_CORNER(pos, i);

	// Draw the contour of the near plane
	DRAW_LINE_CORNERS(0, 1);
	DRAW_LINE_CORNERS(1, 2);
	DRAW_LINE_CORNERS(2, 3);
	DRAW_LINE_CORNERS(3, 0);

	// In case we draw the far plane:
	if(draw_far_plane)
	{
		// Draw the lines linking the near and far planes
		for(uint i=0 ; i < 4 ; i++)
			DRAW_LINE_CORNERS(i, i+4);

		// Draw the contour of the far plane
		DRAW_LINE_CORNERS(0+4, 1+4);
		DRAW_LINE_CORNERS(1+4, 2+4);
		DRAW_LINE_CORNERS(2+4, 3+4);
		DRAW_LINE_CORNERS(3+4, 0+4);
	}

#undef DRAW_LINE_TO_CORNER
#undef DRAW_LINE_CORNERS
}

// User custom data managemment.
// NB: the user is not responsible for deleting the data, and the supplied data
// must be allocated with the "new" operator.
void Light::setUserData(uint index, LightData* user_data)
{
	delete this->user_data[index];
	this->user_data[index] = user_data;

	if(user_data != NULL)
		user_data->setLight(this);
}

LightData* Light::getUserData(uint index)
{
	return user_data[index];
}

const LightData* Light::getUserData(uint index) const
{
	return user_data[index];
}
