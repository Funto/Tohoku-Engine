// Boundaries.h
// This file contains all common NB_MAX_*** constants.

#ifndef BOUNDARIES_H
#define BOUNDARIES_H

// Maximum number of lights
#define NB_MAX_LIGHTS 4	// IMPORTANT: if we want to increase this value over 4, we need to extend
						// the "visibility map" mechanism for the deferred shading renderer.

// Maximum number of VAOs for one Geometry object
// A VAO index has to be below this value.
#define NB_MAX_VAO 4

// Maximum number of profiles for a given material
#define NB_MAX_MATERIAL_PROFILES 10

// Maximum number of LightData-derived objects associated
// with a light.
#define NB_MAX_LIGHT_DATA 10

// Maximum number of texture units we consider possible.
#define NB_MAX_TEXTURE_BINDINGS 32	// OpenGL defines GL_TEXTURE0 to GL_TEXTURE31

// Maximum number of depth layers we consider possible (this is NOT
// the actual number of depth layers we use, we just assume this number
// is lower than this constant).
#define NB_MAX_DEPTH_LAYERS 10

#endif // BOUNDARIES_H
