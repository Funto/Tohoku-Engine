// Profile.cpp

#include "Profile.h"
#include "../../CommonIndices.h"
#include "../../log/Log.h"
#include "GeneralProfile.h"
#include "RaytraceProfile.h"
#include "DepthPeelingProfile.h"
#include <cassert>
using namespace std;

Profile* Profile::create(const string& name, uint* pindex)
{
	Profile* new_profile = NULL;
	uint index = 0;

	if(name == "general")
	{
		new_profile = new GeneralProfile();
		index = GENERAL_PROFILE;
	}
	else if(name == "general_depth_peeling")
	{
		new_profile = new GeneralProfile();
		index = GENERAL_PROFILE_DEPTH_PEELING;
	}
	else if(name == "raytrace")
	{
		new_profile = new RaytraceProfile();
		index = RAYTRACE_PROFILE;
	}
	else if(name == "depth_peeling")
	{
		new_profile = new DepthPeelingProfile();
		index = DEPTH_PEELING_PROFILE;
	}
	else
	{
		logError("tried to create an unknown profile: \"", name, "\"");
		assert(false && "unknown profile");
	}

	if(pindex != NULL)
		*pindex = index;
	return new_profile;
}
