// ray_marching.frag

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
// Uniforms:
uniform sampler2DRect tex_bounce_map_0;
uniform sampler2DRect tex_bounce_map_1;
uniform sampler2DRect tex_screen_positions;
uniform sampler2DRect tex_light_positions;

uniform mat4 light_to_eye_matrix;
uniform mat4 eye_proj_matrix;

// ---------------------------------------------------------------------
#define MAX_DISTANCE 5.0
#define MIN_DISTANCE 0.1	// TODO: this should depend on the angle (ray_dir, normal)
// ---------------------------------------------------------------------

// Fragment shader output:
out vec4 frag_output0;
out vec4 frag_output1;

void main()
{
	// Read the photon power
	vec4 texel_bounce_map_0 = texture(tex_bounce_map_0, gl_FragCoord.xy);
	vec3 power_i = texel_bounce_map_0.xyz;
	
	// If the power is 0, "discard" this ray
	if(dot(power_i, power_i) < 0.01)
	{
		frag_output0 = vec4(0,0,0,0);
		frag_output1 = vec4(0,0,0,0);
	}
	else
	{
		// Read the rest of the bounce map information
		vec4 texel_bounce_map_1 = texture(tex_bounce_map_1, gl_FragCoord.xy);
		vec3 light_space_dir = texel_bounce_map_1.rgb;
		float path_density = texel_bounce_map_1.a;
		
		// Read the position from the light GBuffer
		// TODO: replace this with a recomputation using the Z-buffer, gl_FragCoord.xy,
		// and some matrix!
		vec4 texel_position = texture(tex_light_positions, gl_FragCoord.xy);
		vec3 light_space_pos = texel_position.rgb;
		
		// TODO
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
		
		// BEGIN DEBUG
		frag_output0 = vec4(eye_space_pos, 0);
		frag_output1 = vec4(eye_space_pos + 0.1*eye_space_dir, 0);
		// END DEBUG
		
		// TODO: trace a ray
		vec2 ray_start = 0.5*(first_vertex_ndc.xy + 1.0);	// starting pos, in [0, 1]² coordinates.
		
		// ray_inc: by how much we need to increment start_pos to do one step
		vec2 ray_inc = normalize(second_vertex_ndc.xy - first_vertex_ndc.xy);
		float ray_inc_len = 1.0 / max(ray_inc.x, ray_inc.y);
		ray_inc *= ray_inc_len;	// => the greater of dx and dy is equal to 1
		
		/*for(int i=0 ; i < 1000 ; i++)
		{
			f
		}*/
		
		//frag_output0 = texel_bounce_map_0;
		//frag_output1 = texel_bounce_map_1;
		
		//frag_output0 = vec4(eye_space_pos, 0);
		//frag_output1 = vec4(eye_space_pos + 0.3*eye_space_dir, 0);
		
/*		// TODO: only a test for now
		float depth = texture(tex_screen_positions, gl_FragCoord.xy).z;
		frag_output0.r = depth;
		frag_output1.r = depth;
		
		// TODO: test
		frag_output0.rgb += 0.001 * first_vertex_ndc;
		frag_output1.rgb += 0.001 * second_vertex_ndc;
		*/
	}
	
/*vec4 texel_bounce_map_0 = texelFetch(bounce_map_0, bm_texcoords);
vec3 power_i = texel_bounce_map_0.xyz;	// OK*/

/*// Read the second component of the bounce map
// (ray direction and path density):
vec4 texel_bounce_map_1 = texelFetch(bounce_map_1, bm_texcoords);
vec3 light_space_dir = texel_bounce_map_1.rgb;	// OK
float path_density = texel_bounce_map_1.a;
*/

/*// If the power is 0, "discard" this line by putting it in a non visible position:
if(dot(power_i, power_i) < 0.01)
{
	// We don't need to write to the varyings as all fragments are discarded.

	// Just put the vertices in a clipped position:
	gl_Position = vec4(-2.0, -2.0, 0.0, 1.0);
}*/
}
