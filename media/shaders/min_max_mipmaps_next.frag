// min_max_mipmaps_next.frag

#version 330 core

precision highp float;
precision mediump int;

uniform sampler2DRect tex_prev_layer;

smooth in vec2 texcoords;

out vec4 frag_output;

void main()
{
	frag_output = texture(tex_prev_layer, texcoords);
	//frag_output = vec4(texcoords, 0, 1);
	//frag_output = vec4(1,1,0,1);
}
