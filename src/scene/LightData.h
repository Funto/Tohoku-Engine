// LightData.h
// Interface for any data which can be embedded in a Light

#ifndef LIGHT_DATA_H
#define LIGHT_DATA_H

class Light;

class LightData
{
private:
	const Light* light;	// Pointer to the parent light. There is no ownership.

public:
	enum Type
	{
		SHADOW_MAP,
		GBUFFER,
		BOUNCE_MAP,
		PHOTONS_MAP,
	};

public:
	LightData() : light(NULL) {}
	virtual ~LightData() {}

	virtual Type getType() const = 0;

	const Light* getLight() const {return light;}

	// Setter for use by Light::setUserData(), Light being a friend:
private:
	void setLight(const Light* l) {this->light = l;}

	friend class Light;
};

#endif // LIGHT_DATA_H
