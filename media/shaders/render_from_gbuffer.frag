// render_from_gbuffer.frag

#version 330 core

precision highp float;
precision mediump int;

#include "library/blinn_phong.shader"
#include "library/ashikhmin_shirley.shader"
#include "constants.shader"

// ---------------------------------------------------------------------
// Uniforms:
uniform sampler2DRect tex_positions;
uniform sampler2DRect tex_normals;
uniform sampler2DRect tex_diffuse;
uniform sampler2DRect tex_specular;

// - debug texture:
#ifdef _DEBUG_TEXTURE_
uniform sampler2D tex_debug;
#endif

// - background color:
uniform vec3 back_color;

// - light positions:
#for _NB_LIGHTS_
	uniform vec3 eye_space_light_pos_@;
#endfor

// - visibility maps/shadow maps:
#ifdef _SHADOW_MAPPING_
	#ifdef _VISIBILITY_MAPS_
		uniform sampler2DRect visibility_map;
	#else
		#for _NB_LIGHTS_
			uniform mat4 shadow_matrix_@;
			uniform sampler2DShadow shadow_map_@;
		#endfor
	#endif
#endif

// - "pixels_done":
#ifdef _TEST_PIXELS_DONE_
	uniform sampler2DRect tex_pixels_done;
#endif

// - light colors:
#for _NB_LIGHTS_
	uniform vec3 light_color_@;
#endfor

// ---------------------------------------------------------------------
// Varyings:
smooth in vec2 var_texcoords;

// Fragment shader output:
out vec4 frag_color;

#ifdef _MARK_PIXELS_DONE_
	out vec4 frag_pixel_done;
#endif

// ---------------------------------------------------------------------
// main:
void main()
{
	// Early discarding in case the pixel is already done (for depth peeling):
	#ifdef _TEST_PIXELS_DONE_
		vec4 texel_pixels_done = texture(tex_pixels_done, var_texcoords);
		if(texel_pixels_done.r == 1.0)
			discard;
	#endif

	// Read the normal:
	vec4 texel_normal = texture(tex_normals,   var_texcoords);
	vec3 normal       = texel_normal.xyz;		// eye-space normal

	// Check if there is nothing (normal is 0), and set the back color if it's the case:
	if(dot(normal, normal) < 0.5)
	{
		frag_color = vec4(back_color, 0.0);
		return;
	}

	// Read the diffuse component:
	vec4 texel_diffuse  = texture(tex_diffuse,   var_texcoords);

	// Mark the pixel as "done" if it is opaque (see the explanation of _MARK_PIXELS_DONE_
	// in render_from_gbuffer.vert).
	#ifdef _MARK_PIXELS_DONE_
		if(texel_diffuse.a == 1.0)
			frag_pixel_done.r = 1.0;
	#endif

	// Other texture reads:
	vec4 texel_position = texture(tex_positions, var_texcoords);
	vec4 texel_specular = texture(tex_specular,  var_texcoords);

	// Get values from the read texels:
	vec3 position           = texel_position.xyz;	// eye-space position

	vec4 diffuse_value      = texel_diffuse;		// diffuse value
	vec4 specular_value     = texel_specular;		// specular value
	vec3 view_vec           = normalize(-position);	// eye-space view vector: pos -> eye

	// Compute the final color:
	frag_color = vec4(0.0);

	// Texture fetch from the visibility map:
	#ifdef _VISIBILITY_MAPS_
		vec4 visibility = texture(visibility_map, var_texcoords);
	#endif

	// For each light, add the light's contribution:
	#for _NB_LIGHTS_

		// - compute the light vector:
		vec3 light_vec_@ = normalize(eye_space_light_pos_@ - position);

		// - evaluate the BRDF:
		vec4 brdf_eval_@ = _BRDF_FUNCTION_(
			light_vec_@,
			view_vec,
			normal,
			diffuse_value,
			specular_value);

		// - compute the final contribution of the light (multiply by the light's power):
		vec4 lit_value_@ = vec4(light_color_@ * brdf_eval_@.rgb, 0.0);

		// --------- Shadow mapping ----------
		#ifdef _SHADOW_MAPPING_

			#ifdef _VISIBILITY_MAPS_
				// - add the light's contribution to the fragment:
				frag_color += visibility[@] * lit_value_@;
			#else
				// - compute the coordinates of the corresponding texel in the shadow map:
				vec4 shadow_texcoords_@ = shadow_matrix_@ * vec4(position, 1.0);

				vec3 shadow_texcoords_proj_@ = vec3(
					shadow_texcoords_@.x / shadow_texcoords_@.w,
					shadow_texcoords_@.y / shadow_texcoords_@.w,
					shadow_texcoords_@.z / shadow_texcoords_@.w
				);

				// - determine if the fragment is in the frustum of the light
				float in_frustum_@ = float(	shadow_texcoords_proj_@.s >= 0.0 &&
											shadow_texcoords_proj_@.s <= 1.0 &&
											shadow_texcoords_proj_@.t >= 0.0 &&
											shadow_texcoords_proj_@.t <= 1.0 &&
											shadow_texcoords_proj_@.p >= 0.0 &&
											shadow_texcoords_proj_@.p <= 1.0);

				// - determine if the fragment is visible from the light (shadow mapping):
				float lit_@ = texture(shadow_map_@, shadow_texcoords_proj_@);

				// - add the light's contribution to the fragment:
				frag_color += lit_@ * in_frustum_@ * lit_value_@;

				// - add the ambient component:
				frag_color.rgb += AMBIENT_FACTOR*texel_diffuse.rgb;
			#endif

		#else	// No shadow mapping:
			// - add the light's contribution to the fragment:
			frag_color += lit_value_@;
		#endif
	#endfor

	// Set the output alpha value:
	frag_color.a = texel_diffuse.a;
}
