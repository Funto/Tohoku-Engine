// depth_peeling.vert
// We assume the modelview matrix is orthogonal.
// Supported symbols:
// NORMAL_MAPPING: Supports normal mapping

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
// Uniforms
// - transformation matrices
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;

// ---------------------------------------------------------------------
// Vertex attributes
// - position
in vec3 vertex_position;

// - normal
#ifndef NORMAL_MAPPING
	in vec3 vertex_normal;
#endif

// - tex coords
#ifdef NORMAL_MAPPING
	in vec2 vertex_texcoords;
#endif

// ---------------------------------------------------------------------
// Varying variables
#ifndef NORMAL_MAPPING
	smooth out vec3 var_normal;
#endif

// - interpolated texture coordinates
#ifdef NORMAL_MAPPING
	smooth out vec2 var_texcoords;
#endif

// ---------------------------------------------------------------------
// Main function:
void main()
{
	// Compute the modelview matrix:
	mat4 modelview_matrix = view_matrix * model_matrix;

	// Calculate interpolated normal (NB : assuming modelview_matrix is orthogonal !):
	#ifndef NORMAL_MAPPING
		var_normal = mat3(modelview_matrix) * vertex_normal;
	#endif

	// Calculate world position:
	vec4 eye_space_pos = modelview_matrix * vec4(vertex_position, 1.0);

	// Interpolate texture coordinates
	#ifdef NORMAL_MAPPING
		var_texcoords = vertex_texcoords;
	#endif

	// Calculate the projected position:
	gl_Position = projection_matrix * eye_space_pos;
}
