// Common.h

#ifndef COMMON_H
#define COMMON_H

#include "glm/glm.hpp"
#include "glm/gtc/type_precision.hpp"
#include "utils/AssertStatic.h"
#include <iostream>

typedef glm::gtc::type_precision::uint32   uint;
typedef glm::gtc::type_precision::uint8    uchar;
typedef glm::gtc::type_precision::f32vec3  vec3;
typedef glm::gtc::type_precision::f32vec4  vec4;
typedef glm::gtc::type_precision::f32mat3  mat3;
typedef glm::gtc::type_precision::f32mat4  mat4;
typedef glm::gtc::type_precision::u8vec3   Pixel;

// Check the size of some common types :
inline void __ValidateSizes__()
{
	ASSERT_STATIC(sizeof(uint) == 4);
	ASSERT_STATIC(sizeof(vec3) == 3*sizeof(float));
	ASSERT_STATIC(sizeof(vec4) == 4*sizeof(float));
	ASSERT_STATIC(sizeof(Pixel) == 3*sizeof(unsigned char));
}

// iostream support for GLM types :
// - vec3
inline std::ostream& operator<<(std::ostream& os, const vec3& v)
{
	os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
	return os;
}

// - vec4
inline std::ostream& operator<<(std::ostream& os, const vec4& v)
{
	os << "[" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << "]";
	return os;
}

// - Pixel
inline std::ostream& operator<<(std::ostream& os, const Pixel& p)
{
	os << "[" << int(p.r) << ", " << int(p.g) << ", " << int(p.b) << "]";
	return os;
}

// - mat3
inline std::ostream& operator<<(std::ostream& os, const mat3& mat)
{
	for(uint i=0 ; i < 3 ; i++)
		os << vec3(mat[0][i], mat[1][i], mat[2][i]) << std::endl;

	return os;
}

// - mat4
inline std::ostream& operator<<(std::ostream& os, const mat4& mat)
{
	for(uint i=0 ; i < 4 ; i++)
		os << vec4(mat[0][i], mat[1][i], mat[2][i], mat[3][i]) << std::endl;

	return os;
}

#endif // COMMON_H
