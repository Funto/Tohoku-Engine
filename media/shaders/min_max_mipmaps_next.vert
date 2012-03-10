// min_max_mipmaps_next.vert

#version 330 core

precision highp float;
precision highp int;

uniform vec2 target_size;

in vec2 vertex_position;

smooth out vec2 texcoords;

void main()
{
	texcoords = (vertex_position + vec2(1.0)) * target_size;
	//texcoords = 0.5*texcoords;
	//texcoords = 0.5*(vertex_position + vec2(1.0));
	gl_Position = vec4(vertex_position, 0.0, 1.0);
	//gl_Position = vec4(0.5*vertex_position, 0.0, 1.0);
}
