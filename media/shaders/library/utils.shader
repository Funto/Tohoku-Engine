// utils.shader

#ifndef __UTILS__
#define __UTILS__

// x^2
float pow2(in float x)
{
	return x*x;
}

// x^5
float pow5(in float x)
{
	float x2 = x*x;
	return x2*x2*x;
}

// Fresnel reflection coefficient by Shlick's approximation:
vec3 computeFresnel(in vec3 F0, in float cos_i)
{
	return F0 + (vec3(1.0) - F0) * pow5(1.0 - cos_i);
}

// Sum of the components of a vector
float sum(in vec3 v)
{
	return v.x + v.y + v.z;
}

// Mean of the components of a vector
float mean(in vec3 v)
{
	return sum(v) * 0.333333333;
}

// Normalizes v and returns its length
float unitize(inout vec3 v)
{
	float len = length(v);
	float inv_len = 1.0 / len;
	v *= inv_len;
	return len;
}

// Creates 2 orthonormal tangent vectors X and Y such that
// Z == cross(X, Y)
// Z must be normalized.
void getTangents(in vec3 Z, out vec3 X, out vec3 Y)
{
	// Choose another vector X not colinear to Z:
	float colinear_to_x = float((abs(Z.x) >= 0.9));
	X = vec3(1.0-colinear_to_x, colinear_to_x, 0.0);

	// Remove the part that is parallel to Z and normalize
	X = normalize(X - Z*dot(Z, X));

	// Compute Y:
	Y = cross(Z, X);
}

#endif // __UTILS__
