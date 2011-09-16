// GeneralProfile.h

#ifndef GENERAL_PROFILE_H
#define GENERAL_PROFILE_H

#include "GPUProfile.h"

class GeneralProfile : public GPUProfile
{
public:
	GeneralProfile();
	virtual ~GeneralProfile();

	virtual Type getType() const {return PROFILE_GENERAL;}

protected:
	// Callback called when a new program is created.
	virtual void onNewProgram(glutil::GPUProgram* program);
};

#endif // GENERAL_PROFILE_H
