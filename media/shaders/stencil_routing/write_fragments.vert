// write_fragments.vert

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
// Attributes:
in vec3 vertex_position;

// ---------------------------------------------------------------------
// main:
void main()
{
	gl_Position = vec4(vertex_position, 1.0);
}
