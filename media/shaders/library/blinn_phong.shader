// blinn_phong.shader

#ifndef __BLINN_PHONG__
#define __BLINN_PHONG__

vec4 blinn_phong(	in vec3 light_vec,
					in vec3 view_vec,
					in vec3 normal,
					in vec4 diffuse_value,
					in vec4 specular_value)
{
	vec4 result = vec4(0.0);
	float n_dot_l = max(dot(normal, light_vec), 0.0);

	// Only light the fragment if it is facing the light:
	if(n_dot_l > 0.0)
	{
		// - diffuse:
		vec3 diffuse_term = n_dot_l * diffuse_value.rgb;

		// - specular:
		float specular_exp = specular_value.a;
		vec3 half_vector = normalize(light_vec + view_vec);
		float h_dot_n = max(dot(half_vector, normal), 0.0);
		vec3 specular_term = pow(h_dot_n, specular_exp) * specular_value.rgb;

		// Combine diffuse and specular:
		result.rgb = diffuse_term + specular_term;
	}

	return result;
}

#endif // __BLINN_PHONG__
