// TexunitManager.cpp

#include "TexunitManager.h"
#include "../../log/Log.h"
#include "../../scene/profiles/GPUProfile.h"

// ---------------------------------------------------------------------
TexunitManager::TexunitManager()
{
	clearTexunitsUsage();
}

TexunitManager::~TexunitManager()
{
}

// ---------------------------------------------------------------------
// These functions return one or more free texunits and update texunits_usage[].
uint TexunitManager::getFreeTexunit()
{
	for(uint i=0 ; i < NB_MAX_TEXTURE_BINDINGS ; i++)
	{
		if(texunits_usage[i] == false)
		{
			texunits_usage[i] = true;
			return i;
		}
	}
	logError("no free texunit found!");
	return 0;
}

void TexunitManager::getFreeTexunits(uint* texunits, uint nb_texunits)
{
	uint index = 0;

	// For each texunit to return:
	for(uint i=0 ; i < nb_texunits ; i++)
	{
		// Get the next free texunit
		while(texunits_usage[index] == true)
		{
			index++;
			assert(index < NB_MAX_TEXTURE_BINDINGS && "no free texunit found!");
		}

		// Mark it as used and return it.
		texunits_usage[index] = true;
		texunits[i] = index;
	}
}

// Marks all texunits as unused
void TexunitManager::clearTexunitsUsage()
{
	for(uint i=0 ; i < NB_MAX_TEXTURE_BINDINGS ; i++)
		texunits_usage[i] = false;
}

// Setters:
void TexunitManager::setTexunitsUsage(bool texunits_usage[NB_MAX_TEXTURE_BINDINGS])
{
	for(uint i=0 ; i < NB_MAX_TEXTURE_BINDINGS ; i++)
		this->texunits_usage[i] = texunits_usage[i];
}

void TexunitManager::setTexunitsUsageFromProfile(const GPUProfile* profile)
{
	const GPUProfile::Texture* textures = profile->getTextures();
	uint nb_textures = profile->getNbTextures();

	clearTexunitsUsage();

	for(uint i=0 ; i < nb_textures ; i++)
	{
		uint texunit = textures[i].unit;
		texunits_usage[texunit] = true;
	}
}
