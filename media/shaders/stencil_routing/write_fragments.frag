// write_fragments.frag

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
// Fragment shader output:
out vec4 frag_color;

void main()
{
	frag_color = vec4(1.0, 0.0, 0.0, 0.0);
}
