// general.frag
// See general.vert for explanations about supported symbols.

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
#include "library/blinn_phong.shader"
#include "library/ashikhmin_shirley.shader"
#include "constants.shader"

// ---------------------------------------------------------------------
// Uniforms
// - diffuse:
#ifdef TEXTURE_MAPPING
	uniform sampler2D tex_diffuse;
#else
	uniform vec4 material_diffuse;
#endif

// - specular:
uniform vec4 material_specular;

// - normal texture for normal mapping:
#ifdef NORMAL_MAPPING
	uniform sampler2D tex_normal;
#endif

// - previous depth buffer, for depth peeling:
#ifdef DEPTH_PEELING
	uniform sampler2DRectShadow tex_prev_depth_layer;
#endif

// - shadow maps:
#ifdef _SHADOW_MAPPING_
	#for _NB_LIGHTS_
		uniform sampler2DShadow shadow_map_@;
	#endfor
#endif

// - light colors:
#ifdef _FORWARD_SHADING_
	#for _NB_LIGHTS_
		uniform vec3 light_color_@;
	#endfor
#endif

// - debug
#ifdef _DEBUG_TEXTURE_
	uniform sampler2D tex_debug;
#endif

// ---------------------------------------------------------------------
// Varying variables
// - normals:
#ifndef NORMAL_MAPPING
	smooth in vec3 var_normal;
#endif

// - light vectors:
#ifdef _FORWARD_SHADING_
	#for _NB_LIGHTS_
		smooth in vec3 var_light_vec_@;
	#endfor
#endif

// - interpolated position
smooth in vec3 var_eye_space_pos;

// - interpolated texture coordinates
#if (defined TEXTURE_MAPPING) || (defined NORMAL_MAPPING)
	smooth in vec2 var_texcoords;
#endif

// - texture coordinates for shadow maps
#ifdef _SHADOW_MAPPING_
	#for _NB_LIGHTS_
		smooth in vec4 var_shadow_texcoords_@;
	#endfor
#endif

// ---------------------------------------------------------------------
// Fragment shader output
#ifdef _FORWARD_SHADING_
	out vec4 frag_color;

#else	// deferred shading:
	out vec4 frag_position;		// position.x  | position.y  | position.z  | free
	out vec4 frag_normal;		// normal.x    | normal.y    | normal.z    | free
	out vec4 frag_diffuse;		// diffuse.r   | diffuse.g   | diffuse.b   | alpha value
	out vec4 frag_specular;		// specular.r  | specular.g  | specular.b  | specular exponent
	// Visibility map:
	#ifdef _SHADOW_MAPPING_
		out vec4 frag_visibility;
	#endif
#endif

// ---------------------------------------------------------------------
void main()
{
	// --------- Depth peeling: compare to the previous depth layer ----------
	#ifdef DEPTH_PEELING
		float keep_fragment = texture(tex_prev_depth_layer, gl_FragCoord.xyz);
		if(keep_fragment == 0.0)
			discard;
	#endif

	// --------- Calculate normal ----------
	vec3 normal;
	#ifdef NORMAL_MAPPING
		// TODO: this is WRONG, we do not convert from tangent space
		// to eye space
		//(see http://hacksoflife.blogspot.com/2009/11/per-pixel-tangent-space-normal-mapping.html)
		normal = 2.0*texture(tex_normal, var_texcoords).rgb - vec3(1.0);
	#else
		normal = normalize(var_normal);
	#endif

	// --------- Get the diffuse value ----------
	vec4 diffuse_value;
	#ifdef TEXTURE_MAPPING
		diffuse_value = texture(tex_diffuse, var_texcoords);
	#else
		diffuse_value = material_diffuse;
	#endif

	// --------- Forward shading ----------
	#if   defined _FORWARD_SHADING_
		frag_color = vec4(0.0);

		vec3 view_vec = normalize(-var_eye_space_pos);
	// --------- Deferred shading ----------
	#else
		frag_position = vec4(var_eye_space_pos, 0.0);
		frag_normal   = vec4(normal, 0.0);
		frag_diffuse  = diffuse_value;
		frag_specular = material_specular;
		// frag_visibility is updated later on.
	#endif

	// --------- For each light: ----------
	#for _NB_LIGHTS_
		// --------- Shadow mapping ----------
		// This code is inserted in these cases:
		// - forward shading and shadow mapping
		// - deferred shading and visibility maps
		#if (defined _RENDER_TO_GBUFFER_ && defined _VISIBILITY_MAPS_) || (!defined _RENDER_TO_GBUFFER_ && defined _SHADOW_MAPPING_)
			// - compute the coordinates of the corresponding texel in the shadow map:
			vec3 shadow_texcoords_proj_@ = vec3(
				var_shadow_texcoords_@.s / var_shadow_texcoords_@.q,
				var_shadow_texcoords_@.t / var_shadow_texcoords_@.q,
				var_shadow_texcoords_@.p / var_shadow_texcoords_@.q);

			// - determine if the fragment is in the frustum of the light
			float in_frustum_@ = float(	shadow_texcoords_proj_@.s >= 0.0 &&
										shadow_texcoords_proj_@.s <= 1.0 &&
										shadow_texcoords_proj_@.t >= 0.0 &&
										shadow_texcoords_proj_@.t <= 1.0 &&
										shadow_texcoords_proj_@.p >= 0.0 &&
										shadow_texcoords_proj_@.p <= 1.0);

			// - determine if the fragment is visible from the light (shadow mapping):
			float lit_@ = texture(shadow_map_@, shadow_texcoords_proj_@);
		#endif


		// --------- Deferred shading ----------
		#ifdef _RENDER_TO_GBUFFER_
			// Write into the visibility map:
			#ifdef _VISIBILITY_MAPS_
				frag_visibility[@] = in_frustum_@ * lit_@;
			#endif
		// --------- Forward shading ----------
		#else
			// - compute the light vector:
			vec3 light_vec_@ = normalize(var_light_vec_@);

			// - evaluate the BRDF:
			vec4 brdf_eval_@ = _BRDF_FUNCTION_(
				light_vec_@,
				view_vec,
				normal,
				diffuse_value,
				material_specular);

			// - compute the final contribution of the light (multiply by the light's power):
			vec4 lit_value_@ = vec4(light_color_@ * brdf_eval_@.rgb, brdf_eval_@.a);

			// - add the light's contribution to the fragment:
			#ifdef _SHADOW_MAPPING_
				frag_color += lit_@ * in_frustum_@ * lit_value_@;
			#else
				frag_color += lit_value_@;
			#endif

			// - add the ambient component:
			frag_color += AMBIENT_FACTOR*diffuse_value;
		#endif
	#endfor

	// For forward shading: set the output alpha value:
	#ifdef _FORWARD_SHADING_
		frag_color.a = diffuse_value.a;
	#endif
}
