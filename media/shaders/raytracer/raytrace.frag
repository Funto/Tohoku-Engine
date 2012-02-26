// raytracer/raytrace.frag

#version 330 core

// ---------------------------------------------------------------------
// Default precision:
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
#define Z_ERROR_FACTOR 0.03

// ---------------------------------------------------------------------
// Uniforms:
// - Z-buffers from the point of view of the camera, which we got by depth peeling:
#for _NB_DEPTH_LAYERS_
	uniform sampler2DRect tex_depth_@;
#endfor

// - debug texture:
#ifdef _DEBUG_TEXTURE_
	uniform sampler2DRect tex_debug;
#endif

// - near and far clip distances:
uniform float znear;
uniform float zfar;

// TODO: remove this (debug):
uniform float x_offset;

// ---------------------------------------------------------------------
// Varyings:
noperspective in vec3 var_window_coords;
noperspective in vec3 var_eye_pos_nopersp;
noperspective in float var_inv_w;

#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	//~ smooth in vec3 var_debug_color;
	noperspective in vec3 var_debug_color;
	//~ flat in vec3 var_debug_color;	// the second vertex is taken into account
#endif
flat in vec3 var_power_i;
flat in vec3 var_coming_dir;
flat in float var_path_density;

// ---------------------------------------------------------------------
// Fragment shader output:
#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	out vec4 frag_debug;
#endif
out vec4 frag_position;		// x,     y,     z      free
out vec4 frag_power;		// r,     g,     b,     free
out vec4 frag_coming_dir;	// w_i.x, w_i.y, w_i.z, path_density

// ---------------------------------------------------------------------
void main()
{
	// Round the window coordinates to the closest pixel position:
	vec3 window_coords;
	window_coords.xy = floor(var_window_coords.xy) + vec2(0.5);
	window_coords.z = var_window_coords.z;

	// TODO:
	// Compare each depth layer and the depth of this fragment.

	float depth_val_0 = texture(tex_depth_0, window_coords.xy).r;

	// Compute the perspective-correct eye space coordinates:
	vec3 eye_pos = var_eye_pos_nopersp / var_inv_w;

	// Compute the tolerance in Z used when comparing the Z value of the fragment
	// (in window coordinates) with the values stored in the depth layers.
	// Assuming the depth buffer stores linearly Z_win (Z in window coordinates),
	// we use a tolerance proportional to:
	// dZ_win = (far*near / far-near) / Z_eye^2
	// A difference dZ_eye=1 in Z_eye induces a difference of dZ_win in Z_win
	// (see the derivate of Z_win = f(Z_eye))
	float z_error = (zfar*znear) / (zfar-znear)
					* Z_ERROR_FACTOR / (eye_pos.z * eye_pos.z);

	if(abs(window_coords.z - depth_val_0) < z_error)
	{
		#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
			//frag_debug = vec4(var_debug_color, 0.0);
			frag_debug = vec4(1.0, 0.0, 0.0, 1.0);
		#endif
		frag_position   = vec4(eye_pos,        0.0);
		frag_power      = vec4(var_power_i,    0.0);
		frag_coming_dir = vec4(var_coming_dir, var_path_density);
	}
	else
	{
		#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
			//~ frag_debug = 0.1 * vec4(var_debug_color, 0.0);
			//frag_debug = vec4(0.0);
			frag_debug = vec4(0.0, 1.0, 0.0, 1.0);
			//~ frag_debug = vec4(var_debug_color, 0.0);
		#endif
		frag_position   = vec4(0.0);
		frag_power      = vec4(0.0);
		frag_coming_dir = vec4(0.0);
		discard;
	}

	//~ frag_debug = vec4(	window_coords.z * 0.5,
						//~ z_error * 500.0,
						//~ 0.0, 0.0);
	//~ frag_debug = vec4(window_coords.xy / vec2(_LAYERS_WIDTH_, _LAYERS_HEIGHT_), 0.0, 0.0);

	// Debugging:
	// Test to compare gl_FragCoord and window_coords: ideally, this should be all black:
	//~ frag_debug = 1000.0*vec4(	abs(window_coords.x-gl_FragCoord.x),
									//~ abs(window_coords.y-gl_FragCoord.y),
									//~ abs(window_coords.z-gl_FragCoord.z),
									//~ 0.0);
}
