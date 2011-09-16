// render_to_shadow_map.vert
// Program used for rendering a scene's depth buffer to a texture.
// No supported symbols.

#version 330 core

precision highp float;
precision highp int;

// Uniforms
uniform mat4 modelview_matrix;
uniform mat4 projection_matrix;

// Attributes:
in vec3 vertex_position;

// main:
void main()
{
	gl_Position = projection_matrix * modelview_matrix * vec4(vertex_position, 1.0);
}
