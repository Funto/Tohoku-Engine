// debug/debug_photons_map.frag

#version 330 core

precision highp float;
precision mediump int;

uniform vec4 debug_color;

in vec3 var_color;

out vec4 frag_color;

void main()
{
	frag_color = vec4(var_color, 1);
	//frag_color = debug_color;
}
