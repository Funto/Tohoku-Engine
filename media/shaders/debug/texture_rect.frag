// debug/texture_rect.frag

#version 330 core

precision highp float;
precision mediump int;

uniform sampler2DRect texunit;

smooth in vec2 texcoords;

out vec4 frag_color;

void main()
{
	frag_color = texture(texunit, texcoords);
	//frag_color = vec4(texture(texunit, texcoords).a);
}
