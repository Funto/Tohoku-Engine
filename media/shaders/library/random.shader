// random.shader

#ifndef __RANDOM__
#define __RANDOM__

#include "utils.shader"

vec2 _rand_seed;	// private...

// Initialize the random generator with a 2D position:
void initRand(in vec2 seed)
{
	_rand_seed = seed;
}

// ---------------------------------------------------------------------
// Those functions return uniformly distributed values in the range [0, 1]
float rand1()
{
	float dot_product = dot(_rand_seed, vec2(12.9898,78.233));
	float x = fract(sin(dot_product) * 43758.5453);
	_rand_seed.x = x;
	return x;
}

vec2 rand2()
{
	float dot_product = dot(_rand_seed, vec2(12.9898,78.233));
	vec2 r = vec2(
		fract(sin(dot_product    ) * 43758.5453),
		fract(sin(dot_product*2.0) * 43758.5453));
	_rand_seed = r;
	return r;
}

vec3 rand3()
{
	float dot_product = dot(_rand_seed, vec2(12.9898,78.233));
	vec3 r = vec3(
		fract(sin(dot_product    ) * 43758.5453),
		fract(sin(dot_product*2.0) * 43758.5453),
		fract(sin(dot_product*3.0) * 43758.5453));
	_rand_seed = r.xy;
	return r;
}

// ---------------------------------------------------------------------
// Returns 3D unit vectors distributed according to a
// cosine distribution around the Z axis.
vec3 cosHemiRandom()
{
	vec2 e = rand2();

	// Jensen's method
	float sin_theta = sqrt(1.0 - e.x);
	float cos_theta = sqrt(e.x);
	float phi = 6.28318531 * e.y;

	// We could also use Malley's method (pbrt p.657), since they have the same cost:
	//
	//  r = sqrt(e.x);
	//  t = 2*pi*e.y;
	//  x = cos(t)*r;
	//  y = sin(t)*r;
	//  z = sqrt(1.0 - x*x + y*y);

	return vec3(
		cos(phi) * sin_theta,
		sin(phi) * sin_theta,
		cos_theta);
}

// ---------------------------------------------------------------------
// Same as cosHemiRandom(), except that the Z axis is replaced by
// an arbitrary given vector.
vec3 cosHemiRandom(in vec3 n)
{
	vec3 rand_vec = cosHemiRandom();	// if we get rand_vec == vec3(0.0, 0.0, 1.0),
										// the returned vector is n.

	// Make a coordinate system:
	vec3 Z = n;
	vec3 X, Y;
	getTangents(Z, X, Y);

	return	rand_vec.x * X +
			rand_vec.y * Y +
			rand_vec.z * Z;
}

// ---------------------------------------------------------------------
// Returns 3D unit vectors distributed according to a cosine
// power distribution (pow(cos(theta), k) around the Z axis.
// The method comes from the G3D engine (http://g3d.sourceforge.net)
// The returned vector is closer to vec3(0.0, 0.0, 1.0) as k becomes big.
vec3 cosPowHemiRandom(float k)
{
	vec2 e = rand2();

	float cos_theta = pow(e.x, 1.0 / (k + 1.0));
	float sin_theta = sqrt(1.0 - cos_theta*cos_theta);
	float phi = 6.28318531 * e.y;

	return vec3(
		cos(phi) * sin_theta,
		sin(phi) * sin_theta,
		cos_theta);
}

// ---------------------------------------------------------------------
// Same as cosPowHemiRandom(), except that the Z axis is replaced by
// an arbitrary given vector.
vec3 cosPowHemiRandom(in vec3 n, float k)
{
	vec3 rand_vec = cosPowHemiRandom(k);	// if we get rand_vec == vec3(0.0, 0.0, 1.0),
										// the returned vector is n.

	// Make a coordinate system:
	vec3 Z = n;
	vec3 X, Y;
	getTangents(Z, X, Y);

	return	rand_vec.x * X +
			rand_vec.y * Y +
			rand_vec.z * Z;
}

#endif // __RANDOM__
