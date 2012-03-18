// min_max_mipmaps_first.vert

#version 330 core

precision highp float;
precision highp int;

uniform vec2 source_size;

in vec2 vertex_position;

smooth out vec2 texcoords;

void main()
{
	texcoords = (vertex_position + vec2(1.0)) * source_size;
	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
