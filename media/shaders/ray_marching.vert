// ray_marching.vert

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
// Attributes:
in vec2 vertex_position;

// ---------------------------------------------------------------------
// main:
void main()
{
	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
