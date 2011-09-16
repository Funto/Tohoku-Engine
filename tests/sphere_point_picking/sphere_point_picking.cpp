// sphere_point_picking.cpp

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include "../../src/glm/glm.hpp"
#include "../../src/glm/gtc/type_precision.hpp"
using namespace std;

// ---------------------------------------------------------------------
#define NB_POINTS 1000

// ---------------------------------------------------------------------
typedef glm::gtc::type_precision::uint32   uint;
typedef glm::gtc::type_precision::uint8    uchar;
typedef glm::gtc::type_precision::f32vec2  vec2;
typedef glm::gtc::type_precision::f32vec3  vec3;
typedef glm::gtc::type_precision::f32vec4  vec4;
typedef glm::gtc::type_precision::f32mat3  mat3;
typedef glm::gtc::type_precision::f32mat4  mat4;

// ---------------------------------------------------------------------
float rand1()
{
	return float(rand()) / float(RAND_MAX);
}

vec2 rand2()
{
	return vec2(rand1(), rand1());
}

vec3 rand3()
{
	return vec3(rand1(), rand1(), rand1());
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

	// We could also use Malley's method (pbrt p.657), since they are the same cost:
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
int main()
{
	// Write the data:
	ofstream f_data("data.dat");

	for(uint i=0 ; i < NB_POINTS ; i++)
	{
		vec3 v = cosHemiRandom();
		//vec3 v = cosPowHemiRandom(20.0);
		f_data << v.x << "\t" << v.y << "\t" << v.z << endl;
	}

	f_data.close();

	// Write the commands
	ofstream f_commands("commands.gnp");
	f_commands
		<<
		"splot 'data.dat'\n"
		"pause -1 \"Press enter on the terminal to quit\""
		<< endl;
	//~ f_commands
		//~ <<
		//~ "plot [t=-4:4] sin(t)\n"
		//~ "pause -1 \"Press enter on the terminal to quit\""
		//~ << endl;
	f_commands.close();

	// Execute GNUPlot
	system("gnuplot commands.gnp");
	return 0;
}
