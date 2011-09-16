// DepthPeelingProfile.h

#ifndef DEPTH_PEELING_PROFILE_H
#define DEPTH_PEELING_PROFILE_H

#include "GPUProfile.h"

class DepthPeelingProfile : public GPUProfile
{
public:
	DepthPeelingProfile();
	virtual ~DepthPeelingProfile();

	virtual Type getType() const {return PROFILE_DEPTH_PEELING;}

	virtual void onNewProgram(glutil::GPUProgram *program);
};

#endif // DEPTH_PEELING_PROFILE_H
