// GLRaytracerConfig.h
// This file contains the common configuration definitions
// for the GLRaytracer and TextureReducer classes.
// It should only be included by GLRaytracer.cpp, TextureReducer.cpp and PhotonVolumesRenderer.cpp

#ifndef GL_RAYTRACER_CONFIG_H
#define GL_RAYTRACER_CONFIG_H

// FBO size:
#define FBO_WIDTH  1024
#define FBO_HEIGHT 1024

// Debug flags:

//#define DEBUG_NB_LINES 1000
//#define DEBUG_RENDER_TO_SCREEN
//#define DEBUG_USE_EYE_SPACE_LINES
//#define DEBUG_NO_TEXTURE_REDUCE
//#define DEBUG_USE_DEBUG_FBO_ATTACHMENT	// if defined, var_debug is outputed by the fragment shader
										// to either the screen or id_debug_target
										// (depending on whether DEBUG_RENDER_TO_SCREEN is defined or not)

// Fragment shader output locations (for writing to the FBO owned by GLRaytracer):
#ifdef DEBUG_USE_DEBUG_FBO_ATTACHMENT
	#define FRAG_DATA_DEBUG 0
#else
	#define FRAG_DATA_DEBUG -1
#endif

#define FRAG_DATA_POSITION      (FRAG_DATA_DEBUG + 1)
#define FRAG_DATA_POWER         (FRAG_DATA_DEBUG + 2)
#define FRAG_DATA_COMING_DIR    (FRAG_DATA_DEBUG + 3)

#endif // GL_RAYTRACER_CONFIG_H
