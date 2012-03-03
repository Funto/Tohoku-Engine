// CommonIndices.h
// We group indices for the VAOs, the custom profiles for materials...etc in a single header file
// for avoiding several renderers to use the same indices at the same time.

#ifndef COMMON_INDICES_H
#define COMMON_INDICES_H

#include "Boundaries.h"
#include "utils/AssertStatic.h"

// VAO indices:
#define VAO_INDEX_RASTER        0
#define VAO_INDEX_RASTER_SHADOW 1
#define VAO_INDEX_DEPTH_PEELING 2
#define VAO_INDEX_BOUNCE_MAP    3

inline void __ValidateVAOIndices__()
{
	ASSERT_STATIC(VAO_INDEX_RASTER_SHADOW < NB_MAX_VAO);
}

// Material profile indices:

#define GENERAL_PROFILE               0
#define GENERAL_PROFILE_DEPTH_PEELING 1
#define RAYTRACE_PROFILE              2
#define DEPTH_PEELING_PROFILE         3

inline void __ValidateCustomProfileIndices__()
{
	ASSERT_STATIC(DEPTH_PEELING_PROFILE < NB_MAX_MATERIAL_PROFILES);
}

// Light custom data indices:

#define LIGHT_DATA_SHADOW_MAP  0
#define LIGHT_DATA_GBUFFER     1
#define LIGHT_DATA_BOUNCE_MAP  2
#define LIGHT_DATA_PHOTONS_MAP 3

inline void _ValidateLightDataIndices__()
{
	ASSERT_STATIC(LIGHT_DATA_BOUNCE_MAP < NB_MAX_LIGHT_DATA);
}

#endif // COMMON_INDICES_H
