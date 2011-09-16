// render_to_shadow_map.frag

#version 330 core

precision highp float;
precision mediump int;

// Fragment shader output:
out vec4 frag_color;	// unused, necessary for the completeness of the FBO

// main:
void main()
{
	frag_color = vec4(0.0, 0.0, 0.0, 0.0);
}
