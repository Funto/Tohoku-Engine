// Light.h

#ifndef LIGHT_H
#define LIGHT_H

#include "../Boundaries.h"
#include "Element.h"
#include <string>

class TiXmlElement;
class LightData;

class Light : public Element
{
private:
	vec3 color;
	float angle;	// total angle (not half-angle), in degrees
					// 0.0 means omnidirectional light (shadow cubemaps for those are NOT supported!)
	uint shadow_map_size;	// size of the shadow map (e.g. 512 for 512x512)
	uint bounce_map_size;	// size of the bounce map

	LightData* user_data[NB_MAX_LIGHT_DATA];	// user custom data

public:
	Light();
	virtual ~Light();

	// Setters / getters:
	// - color
	void        setColor(const vec3& color) {this->color = color;}
	const vec3& getColor() const            {return color;}

	// - angle of view from the light:
	void  setAngle(float angle) {this->angle = angle;}
	float getAngle() const      {return angle;}

	// - direction of the light:
	vec3 getDirection() const {return -getOrientation()[2];}

	// - shadow map size:
	uint getShadowMapSize() const {return shadow_map_size;}

	// - bounce map size:
	uint getBounceMapSize() const {return bounce_map_size;}

	// Load from an XML file:
	bool loadFromXML(const std::string& filename);

	// Matrix computation:
	mat4 computeViewMatrix() const;
	mat4 computeProjectionMatrix() const;

	// Debug drawing of the light:
	void debugDrawFrustum(const mat4& proj_matrix,
						  const mat4& view_matrix,
						  const vec3& color=vec3(1.0, 1.0, 1.0),
						  bool draw_far_plane=true);

	// User custom data managemment.
	// NB: the user is not responsible for deleting the data, and the supplied data
	// must be allocated with the "new" operator.
	void             setUserData(uint index, LightData* user_data);
	LightData*       getUserData(uint index);
	const LightData* getUserData(uint index) const;
};

#endif // LIGHT_H
