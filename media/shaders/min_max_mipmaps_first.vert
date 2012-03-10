// min_max_mipmaps_first.vert

#version 330 core

precision highp float;
precision highp int;

in vec2 vertex_position;

smooth out vec2 texcoords;

void main()
{
	texcoords = (0.5*(vertex_position + vec2(1.0))) * vec2(_TARGET_WIDTH_, _TARGET_HEIGHT_);
	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
