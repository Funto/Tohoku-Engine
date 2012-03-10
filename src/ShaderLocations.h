// ShaderLocations.h
// We group in this file the vertex attribute locations and the fragment data locations.

#ifndef SHADER_LOCATIONS_H
#define SHADER_LOCATIONS_H

// ---------------------------------------------------------------------
// GENERAL PROFILE:
// Vertex attributes:
#define GENERAL_PROFILE_ATTRIB_POSITION  0
#define GENERAL_PROFILE_ATTRIB_NORMAL    1
#define GENERAL_PROFILE_ATTRIB_TEXCOORDS 2

// Fragment data (fragment shader output):
// - forward rendering
#define GENERAL_PROFILE_FRAG_DATA_COLOR  0

// - deferred shading (_RENDER_TO_GBUFFER_ defined)
#define GENERAL_PROFILE_FRAG_DATA_POSITION   0
#define GENERAL_PROFILE_FRAG_DATA_NORMAL     1
#define GENERAL_PROFILE_FRAG_DATA_DIFFUSE    2
#define GENERAL_PROFILE_FRAG_DATA_SPECULAR   3
#define GENERAL_PROFILE_FRAG_DATA_VISIBILITY 4

// ---------------------------------------------------------------------
// DEPTH PEELING PROFILE:
// Vertex attributes:
#define DEPTH_PEELING_ATTRIB_POSITION  0
#define DEPTH_PEELING_ATTRIB_NORMAL    1
#define DEPTH_PEELING_ATTRIB_TEXCOORDS 2

// Fragment data (fragment shader output):
#define DEPTH_PEELING_FRAG_DATA_NORMAL 0

// ---------------------------------------------------------------------
// SHADOW MAPS:
#define SHADOW_MAP_ATTRIB_POSITION 0

#define SHADOW_MAP_FRAG_DATA_COLOR 0

// ---------------------------------------------------------------------
// BOUNCE MAPS:
#define BOUNCE_MAP_ATTRIB_POSITION 0

#define BOUNCE_MAP_FRAG_DATA_OUTPUT0 0
#define BOUNCE_MAP_FRAG_DATA_OUTPUT1 1

// ---------------------------------------------------------------------
// PHOTONS MAPS:
#define PHOTONS_MAP_ATTRIB_POSITION 0

#define PHOTONS_MAP_FRAG_DATA_OUTPUT0 0
#define PHOTONS_MAP_FRAG_DATA_OUTPUT1 1

// ---------------------------------------------------------------------
// MIN-MAX MIPMAPS
#define MIN_MAX_MIPMAPS_ATTRIB_POSITION 0

#define MIN_MAX_MIPMAPS_FRAG_DATA_OUTPUT 0

// Debug:
#define DEBUG_PHOTONS_MAP_ATTRIB_POSITION 0

#define DEBUG_PHOTONS_MAP_FRAG_DATA_COLOR 0

#endif // SHADER_LOCATIONS_H
