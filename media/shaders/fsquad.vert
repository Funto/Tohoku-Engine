// fsquad.vert

#version 330 core

precision highp float;
precision highp int;

in vec2 vertex_position;
in vec2 vertex_texcoords;

smooth out vec2 var_texcoords;

void main()
{
	var_texcoords = vertex_texcoords;
	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
