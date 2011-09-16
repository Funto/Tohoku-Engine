// raytracer/raytrace.vert
// Supported symbols:
// _FROM_BOUNCE_MAP_                 : the source of data is the bounce map
// _TARGET_WIDTH_, _TARGET_HEIGHT_   : size of the output texture (floats)
// _LAYERS_WIDTH_, _LAYERS_HEIGHT_   : size of the depth layers (floats)
// _NB_DEPTH_LAYERS_                 : number of depth layers,
//                                     including the visible geometry
// _DEBUG_TEXTURE_                   : Adds a debug texture
// _DEBUG_USE_EYE_SPACE_LINES_       : Renders the lines in eye space instead
//                                     of aligning them to the target framebuffer
// _DEBUG_DONT_DISCARD_FRAGMENTS_    : Fragments are not discarded in the fragment
//                                     shader, but the fragment shader outputs black instead.
// _DEBUG_USE_DEBUG_FBO_ATTACHMENT_  : var_debug_color is used and outputed to frag_debug.

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
#include "../library/utils.shader"
#include "../library/clipping.shader"

#define MAX_DISTANCE 5.0
//~ #define MAX_DISTANCE 1.0
//~ #define MAX_DISTANCE 0.5

#define MIN_DISTANCE 0.1	// TODO: this should depend on the angle (ray_dir, normal)
//~ #define MIN_DISTANCE 0.0

// ---------------------------------------------------------------------
// Uniforms:
uniform sampler2DRect bounce_map_0;
uniform sampler2DRect bounce_map_1;
uniform sampler2DRect tex_light_positions;	// positions from the light's GBuffer

uniform int bounce_map_size;
uniform mat4 light_to_eye_matrix;
uniform mat4 eye_proj_matrix;

uniform float x_offset;
uniform int instance_id_offset;

// ---------------------------------------------------------------------
// Attributes:
in vec2 vertex_position;

// ---------------------------------------------------------------------
// Varyings:
// We do not want to use "smooth" because this would use division by gl_Position.w
// which is not the position of the line in clip space, as it is changed because
// we want to align the lines on the framebuffer for the texture reduce operation which comes next.
noperspective out vec3 var_window_coords;
noperspective out vec3 var_eye_pos_nopersp;
noperspective out float var_inv_w;

#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	//~ smooth out vec3 var_debug_color;
	noperspective out vec3 var_debug_color;
	//~ flat out vec3 var_debug_color;	// the second vertex is taken into account
#endif
flat out vec3 var_power_i;
flat out vec3 var_coming_dir;
flat out float var_path_density;

void lineClip(inout vec4 p0, inout vec4 p1);

// ---------------------------------------------------------------------
void main()
{
	// Calculate the texture coordinates of the corresponding sample in the
	// bounce map:
	int instance_id = gl_InstanceID + instance_id_offset;
	ivec2 bm_texcoords;
	bm_texcoords.y = instance_id / bounce_map_size;
	bm_texcoords.x = instance_id - bm_texcoords.y * bounce_map_size;	// OK

	// Read the first component of the bounce map:
	vec4 texel_bounce_map_0 = texelFetch(bounce_map_0, bm_texcoords);
	vec3 power_i = texel_bounce_map_0.xyz;	// OK

	// If the power is 0, "discard" this line by putting it in a non visible position:
	if(dot(power_i, power_i) < 0.01)
	{
		// We don't need to write to the varyings as all fragments are discarded.

		// Just put the vertices in a clipped position:
		gl_Position = vec4(-2.0, -2.0, 0.0, 1.0);
	}
	// If we do not discard the line:
	else
	{
		// Read the second component of the bounce map
		// (ray direction and path density):
		vec4 texel_bounce_map_1 = texelFetch(bounce_map_1, bm_texcoords);
		vec3 light_space_dir = texel_bounce_map_1.rgb;	// OK
		float path_density = texel_bounce_map_1.a;

		// Read the position from the light GBuffer
		// TODO: replace this with a recomputation using the Z-buffer, bm_texcoords,
		// and some matrix!
		vec4 texel_position = texelFetch(tex_light_positions, bm_texcoords);
		vec3 light_space_pos = texel_position.rgb;	// OK

		// Change the position from light space to eye space:
		// NB: this is the position of the intersection, not the position of any vertex!
		vec3 eye_space_pos = vec3(light_to_eye_matrix * vec4(light_space_pos, 1.0));	// OK

		// Compute the eye space direction of the ray.
		// NB: we assume the transformation matrices are orthogonal,
		// in which case the transpose inverse of the matrix is equal
		// to the matrix itself.
		// See the reasons for gl_NormalMatrix in previous OpenGL versions:
		// http://www.lighthouse3d.com/opengl/glsl/index.php?normalmatrix
		vec3 eye_space_dir = mat3(light_to_eye_matrix) * light_space_dir;

		// Compute the coordinates of the first and second vertices:
		vec3 first_vertex_pos      = eye_space_pos + MIN_DISTANCE * eye_space_dir;
		vec3 second_vertex_pos     = eye_space_pos + MAX_DISTANCE * eye_space_dir;

		// Project the coordinates:
		vec4 first_vertex_proj_pos  = eye_proj_matrix * vec4(first_vertex_pos,  1.0);
		vec4 second_vertex_proj_pos = eye_proj_matrix * vec4(second_vertex_pos, 1.0);

		// Clipping:
		//~ lineClip(first_vertex_proj_pos, second_vertex_proj_pos);

		// Perspective division -> normalized device coordinates:
		vec3 first_vertex_ndc  = first_vertex_proj_pos.xyz  / first_vertex_proj_pos.w;
		vec3 second_vertex_ndc = second_vertex_proj_pos.xyz / second_vertex_proj_pos.w;

		// Compute the length of the line:
		vec2 delta = abs(second_vertex_ndc.xy - first_vertex_ndc.xy);
		float line_length = max(delta.x, delta.y * (_LAYERS_HEIGHT_ / _LAYERS_WIDTH_));

		// Get the clip space pos, NDC, eye space position and final X output of the
		// vertex shader for the current vertex:
		vec3 ndc;
		vec4 proj_pos;
		vec3 vertex_eye_space_pos;
		float final_x;
		if(vertex_position.x < 0.0)
		{
			ndc = first_vertex_ndc;
			proj_pos = first_vertex_proj_pos;
			vertex_eye_space_pos = first_vertex_pos;
			final_x = -1.0 + x_offset;
		}
		else
		{
			ndc = second_vertex_ndc;
			proj_pos = second_vertex_proj_pos;
			vertex_eye_space_pos = second_vertex_pos;
			final_x = -1.0 + + x_offset + line_length;
		}

		// Viewport mapping -> window coordinates
		// + interpolate the window coordinates
		var_window_coords = 0.5 * (ndc + vec3(1.0))
							 * vec3(_LAYERS_WIDTH_, _LAYERS_HEIGHT_, 1.0);

		// Interpolate 1/w for simulating perspective division:
		var_inv_w = 1.0 / proj_pos.w;

		// Manual perspective-correct interpolation of the position in eye space:
		var_eye_pos_nopersp = vertex_eye_space_pos * var_inv_w;

		// Send the other varyings:
		var_power_i = power_i;
		var_coming_dir = -eye_space_dir;
		var_path_density = texel_bounce_map_1.a;

		// Debugging:
		#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
			//~ if(gl_InstanceID % 2 == 0)
				//~ var_debug_color = vec3(1.0, 0.0, 0.0);
			//~ else
				//~ var_debug_color = vec3(0.0, 1.0, 0.0);

			var_debug_color = power_i;

			//~ if(gl_InstanceID == 97)
			//~ {
				//~ var_debug_color = power_i;
				//~ if(vertex_position.x < 0.0)
					//~ var_debug_color = vec3(0.0, 0.5, 0.0);
				//~ else
					//~ var_debug_color = vec3(0.5, 0.0, 0.0);
			//~ }
			//~ else
				//~ var_debug_color = vec3(0.0);

			//~ if(gl_InstanceID == 2)
			//~ {
				//~ if(vertex_position.x < 0.0)
					//~ var_debug_color = vec3(0.0, 0.5, 0.0);
				//~ else
					//~ var_debug_color = vec3(0.5, 0.0, 0.0);
			//~ }
			//~ else
				//~ var_debug_color = vec3(0.0);
		#endif

		// Set the position of the vertices:
		#ifdef _DEBUG_USE_EYE_SPACE_LINES_
			gl_Position = proj_pos;
		#else
			// TODO: WHY do we have to add 1 to gl_InstanceID to have something visible??
			gl_Position = vec4(	final_x,
								-1.0 + float(gl_InstanceID+1) * (2.0/_LAYERS_HEIGHT_),
								0.0,
								1.0);
		#endif
	}
}
