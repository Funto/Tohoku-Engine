// photon_volumes.frag

#version 330 core

// ---------------------------------------------------------------------
// Default precision:
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
#include "library/utils.shader"
#include "library/blinn_phong.shader"
#include "library/ashikhmin_shirley.shader"
#include "constants.shader"

// ---------------------------------------------------------------------
// Uniforms:
uniform sampler2DRect tex_gbuffer_position;
uniform sampler2DRect tex_gbuffer_normal;
uniform sampler2DRect tex_gbuffer_diffuse;
uniform sampler2DRect tex_gbuffer_specular;
uniform sampler1D tex_kernel;

// ---------------------------------------------------------------------
// Varyings:
flat in vec3 var_eye_space_photon_pos;
flat in vec3 var_eye_space_photon_normal;
flat in vec3 var_scaled_power;
flat in vec3 var_normal;
#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	flat in vec3 var_debug_color;
#endif
flat in vec3 var_coming_dir;
flat in float var_inv_major_radius;
flat in float var_major_radius;

// ---------------------------------------------------------------------
// Fragment shader output:
out vec4 frag_color;

void main_back()
{
	frag_color = vec4(var_scaled_power, 0.0);
}

// ---------------------------------------------------------------------
void main()
{
	// Texture fetches:
	vec4 texel_position = texture(tex_gbuffer_position, gl_FragCoord.xy);
	vec4 texel_normal   = texture(tex_gbuffer_normal,   gl_FragCoord.xy);
	vec4 texel_diffuse  = texture(tex_gbuffer_diffuse,  gl_FragCoord.xy);
	vec4 texel_specular = texture(tex_gbuffer_specular, gl_FragCoord.xy);

	vec3 eye_space_pos = texel_position.rgb;
	vec3 eye_space_normal = texel_normal.rgb;

	vec3 photon_to_point = eye_space_pos - var_eye_space_photon_pos;
	float dist_photon_to_point = length(photon_to_point);

	vec3 w_i = var_coming_dir + photon_to_point;

	float n_dot_l = dot(eye_space_normal, w_i);

	// Discard the cases where:
	// - the point is not facing the "light":
	// - the point is out of the visibility of the kernel
	if(n_dot_l <= 0.0 || dist_photon_to_point > var_major_radius)
		discard;

	// BRDF evaluation:
	// BEGIN BOUM
	vec4 brdf_eval = _BRDF_FUNCTION_(	w_i,
										-eye_space_pos,
										eye_space_normal,
										texel_diffuse,
										texel_specular
									);

//	vec4 brdf_eval = 0.3*vec4(texel_diffuse.rgb, 0);
	// END BOUM
	

	// Fetch the kernel value:
	float relative_dist = dist_photon_to_point * var_inv_major_radius;
	float falloff = texture(tex_kernel, relative_dist, 0).r;

	frag_color = vec4(var_scaled_power * vec3(brdf_eval) * falloff, 0.0);
}
