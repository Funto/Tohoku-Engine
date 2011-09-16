// debug/unicolor.frag

#version 330 core

// ---------------------------------------------------------------------
// Default precision:
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
flat in vec3 color;

out vec4 frag_color;

// ---------------------------------------------------------------------
void main()
{
	frag_color = vec4(color, 0.0);
}
