// ashikhmin_shirley.shader
// This is the isotropic version of the Ashikhmin-Shirley BRDF.

#ifndef __ASHIKHMIN_SHIRLEY__
#define __ASHIKHMIN_SHIRLEY__

#include "utils.shader"

vec4 ashikhmin_shirley(	in vec3 light_vec,
						in vec3 view_vec,
						in vec3 normal,
						in vec4 diffuse_value,
						in vec4 specular_value)
{
	vec3 half_vector = normalize(light_vec + view_vec);

	float n_dot_l = max(dot(normal,   light_vec),   0.0);
	float n_dot_v = max(dot(normal,   view_vec),    0.0);
	float n_dot_h = max(dot(normal,   half_vector), 0.0);
	float v_dot_h = max(dot(view_vec, half_vector), 0.0);

	float PI = 3.14159265358979323846264;
	vec3 one = vec3(1.0);

	// - diffuse term:
	vec3 diffuse_term =	(28.0/(23.0*PI)) *
						vec3(diffuse_value) *
						(one - vec3(specular_value)) *
						(1.0 - pow5(1.0 - 0.5*n_dot_v)) *
						(1.0 - pow5(1.0 - 0.5*n_dot_l));

	// - specular term:
	float specular_exp = specular_value.a;
	float numerator    =	pow(n_dot_h, specular_exp);
	float denominator  =	v_dot_h * max(n_dot_l, n_dot_v);

	vec3 specular_term =	(specular_exp + 1.0)/(8.0*PI) *
							(numerator / denominator) *
							computeFresnel(specular_value.rgb, v_dot_h);

	// Combine diffuse and specular:
	return vec4(diffuse_term + specular_term, 0.0);
}

#endif // __ASHIKHMIN_SHIRLEY__
