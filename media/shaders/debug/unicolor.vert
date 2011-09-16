// debug/unicolor.vert

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
uniform mat4 proj_matrix;
uniform mat4 view_matrix;

in vec3 vertex_position;
in vec3 vertex_color;

flat out vec3 color;

// ---------------------------------------------------------------------
void main()
{
	color = vertex_color;
	gl_Position = proj_matrix * view_matrix * vec4(vertex_position, 1.0);
}
