// photon_volumes.vert
// Supported symbols:
// _DEBUG_USE_DEBUG_FBO_ATTACHMENT_                    : tex_debug is provided
// _LAYERS_WIDTH_, _LAYERS_HEIGHT_                     : size of the depth layers
// _INTERSECTION_MAP_WIDTH_, _INTERSECTION_MAP_HEIGHT_ : size of the intersection map
// _BRDF_FUNCTION_                                     : Name of the BRDF function to use
//                                                       (e.g. "blinn_phong")

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
#include "constants.shader"

// BEGIN VALUES FOR THE CAUSTICS EXPERIMENT
//~ // Upper and lower sizes of the major axis of a photon volume in eye space length units
//~ #define MAX_MAJOR_RADIUS 0.2
//~ #define MIN_MAJOR_RADIUS 0.1
//~
//~ #define OVERSIZE (1.0 + 0.5)	// factor used to oversize the photon volumes...
//~
//~ // TODO: put a physically correct value
//~ //#define PHOTON_ENERGY_SCALE_FACTOR (0.5 / M_PI)
//~ #define PHOTON_ENERGY_SCALE_FACTOR (0.4 / M_PI)
// END VALUES FOR THE CAUSTICS EXPERIMENT

// BEGIN PREVIOUS VALUES
// Upper and lower sizes of the major axis of a photon volume in eye space length units
#define MAX_MAJOR_RADIUS 1.0
#define MIN_MAJOR_RADIUS 0.5

#define OVERSIZE (1.0 + 0.5)	// factor used to oversize the photon volumes...

// TODO: put a physically correct value
//#define PHOTON_ENERGY_SCALE_FACTOR (0.5 / M_PI)
#define PHOTON_ENERGY_SCALE_FACTOR (0.2 / M_PI)
// END PREVIOUS VALUES

// ---------------------------------------------------------------------
// Uniforms:
// - matrices:
uniform mat4 eye_view_matrix;
uniform mat4 eye_proj_matrix;

// - from the intersection map:
#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	uniform sampler2DRect tex_debug;
#endif
uniform sampler2DRect tex_power;
uniform sampler2DRect tex_position;
uniform sampler2DRect tex_coming_dir;

// - from the camera GBuffer:
uniform sampler2DRect tex_gbuffer_normal;

// ---------------------------------------------------------------------
// Attributes:
in vec3 vertex_position;

// ---------------------------------------------------------------------
// Varyings:
flat out vec3 var_eye_space_photon_pos;
flat out vec3 var_eye_space_photon_normal;
flat out vec3 var_scaled_power;
flat out vec3 var_normal;
#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	flat out vec3 var_debug_color;
#endif
flat out vec3 var_coming_dir;
flat out float var_major_radius;
flat out float var_inv_major_radius;

// ---------------------------------------------------------------------
void main()
{
	ivec2 texcoords;
	texcoords.x = gl_InstanceID/_INTERSECTION_MAP_HEIGHT_;
	texcoords.y = gl_InstanceID - texcoords.x * _INTERSECTION_MAP_HEIGHT_;

	// Read the power of the photon:
	vec4 texel_power = texelFetch(tex_power, texcoords);
	vec3 power = texel_power.rgb;

	// If there is no power, "discard" the volume (draw it somewhere it is clipped):
	if(dot(power, power) < 0.1)
	{
		gl_Position = vec4(-2.0, -2.0, 0.0, 1.0);
	}
	// If it is not clipped:
	else
	{
		// Read the photon's coming direction and path density:
		vec4 texel_coming_dir = texelFetch(tex_coming_dir, texcoords);
		float path_density = texel_coming_dir.a;

		// Read the photon's eye-space position:
		vec4 texel_position = texelFetch(tex_position, texcoords);
		vec4 photon_eye_space_pos = vec4(texel_position.rgb, 1.0);

		// Compute the window coordinates, which we use for fetching
		// from the camera GBuffer:
		vec4 photon_proj_pos = eye_proj_matrix * photon_eye_space_pos;
		vec3 photon_ndc = photon_proj_pos.xyz / photon_proj_pos.w;
		vec2 photon_wincoords = (vec2(0.5) + 0.5*photon_ndc.xy)
								* vec2(_LAYERS_WIDTH_, _LAYERS_HEIGHT_);
		ivec2 photon_wincoords_int = ivec2(photon_wincoords);

		// Fetch the normal to the surface where the photon resides from the GBuffer:
		vec3 photon_normal = texelFetch(tex_gbuffer_normal, photon_wincoords_int).rgb;

		float major_radius = mix(MAX_MAJOR_RADIUS, MIN_MAJOR_RADIUS, path_density);

		// Send varyings to the fragment shader:
		#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
			//~ var_debug_color = vec3(photon_wincoords / vec2(_LAYERS_WIDTH_, _LAYERS_HEIGHT_), 0.0);
			//~ var_debug_color = photon_normal;
			//~ var_debug_color = texelFetch(tex_gbuffer_normal, ivec2(320, 240)).rgb;
			//~ var_debug_color = texelFetch(tex_debug, texcoords).rgb;
			//~ var_debug_color = texelFetch(tex_position, texcoords).rgb;
			var_debug_color = vec3(path_density);
		#endif

		var_eye_space_photon_pos = photon_eye_space_pos.xyz;
		var_eye_space_photon_normal = photon_normal;
		var_scaled_power = power * PHOTON_ENERGY_SCALE_FACTOR;	// Normalize the photon power
		var_normal = photon_normal;
		var_coming_dir = texel_coming_dir.rgb;
		var_major_radius = major_radius;
		var_inv_major_radius = 1.0 / major_radius;

		// Compress the sphere to an ellipsoid along the minor axis (which is photon_normal):
		vec3 compressed_vertex =
			OVERSIZE *
			major_radius *
			(vertex_position - K_SQUASH * photon_normal * dot(vertex_position, photon_normal));

		// Explanation:
		// - for a vertex in the plane orthogonal to photon_normal,
		//   dot(vertex_position, photon_normal) == 0.0
		//   => compressed_vertex == kernel_major_radius * vertex_position
		// - for a vertex in the line of photon_normal:
		//   dot(vertex_position, photon_normal) == 1.0
		//   => compressed_vertex == kernel_major_radius * (vertex_position - K_SQUASH * photon_normal)

		// Compute the final vertex position:
		gl_Position =	eye_proj_matrix
						* (photon_eye_space_pos +  vec4(compressed_vertex, 0.0));
	}
}
