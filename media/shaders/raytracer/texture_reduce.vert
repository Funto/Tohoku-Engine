// raytracer/texture_reduce.vert
// Supported symbols:
// _TARGET_WIDTH_, _TARGET_HEIGHT_   : size of the output texture (int)
// _DEBUG_USE_DEBUG_FBO_ATTACHMENT_  : there is a debug texture as attachment 0 of the FBO

#version 330 core

precision highp float;
precision highp int;

// ---------------------------------------------------------------------
// Vertex attributes:
in vec2 vertex_position;

void main()
{
	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
