// TexunitManager.h
// This class manages texture units usage.

#ifndef TEXUNIT_MANAGER_H
#define TEXUNIT_MANAGER_H

#include "../../Common.h"
#include "../../Boundaries.h"

class GPUProfile;

class TexunitManager
{
private:
	// Internal array describing the texunits usage.
	// "false" means the texunit is unused
	// "true" means a texture is bound to the given texunit.
	bool texunits_usage[NB_MAX_TEXTURE_BINDINGS];

public:
	TexunitManager();
	virtual ~TexunitManager();

	// These functions return one or more free texunits and update texunits_usage[].
	uint getFreeTexunit();
	void getFreeTexunits(uint* texunits, uint nb_texunits);

	// Marks all texunits as unused
	void clearTexunitsUsage();

	// Setters;
	void setTexunitsUsage(bool texunits_usage[NB_MAX_TEXTURE_BINDINGS]);
	void setTexunitsUsageFromProfile(const GPUProfile* profile);

	// Getter:
	bool* getTexunitsUsage()             {return texunits_usage;}
	const bool* getTexunitsUsage() const {return texunits_usage;}
};

#endif // TEXUNIT_MANAGER_H
