// constants.shader

#ifndef _CONSTANTS_SHADER_
#define _CONSTANTS_SHADER_

// ---------------------------------------------------------------------
#define M_PI 3.14159265358979323846

// ---------------------------------------------------------------------
// Direct illumination:

#define AMBIENT_FACTOR 0.2

// ---------------------------------------------------------------------
// Photon volumes:

// oblate spheroid: k = (maj - min) / maj = 1.0 - min/maj:
// See: http://en.wikipedia.org/wiki/Oblate_spheroid
#define K_SQUASH  (1.0 - 0.5/1.0)
#define K_SQUASH2 (K_SQUASH * K_SQUASH)

#endif // _CONSTANTS_SHADER_
