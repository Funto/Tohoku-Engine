// min_max_mipmaps.frag

#version 330 core

precision highp float;
precision mediump int;

uniform sampler2DRect tex_position;

smooth in vec2 texcoords;

out vec4 frag_output;

void main()
{
	frag_output = vec4(-0.1*texture(tex_position, texcoords).zz, 0, 1);
}
