// render_from_gbuffer.vert
// Program used for rendering a scene in a full-screen quad using
// a G-buffer and light positions.
// Supported symbols:
// _NB_LIGHTS_           : Number of lights to support
// _SHADOW_MAPPING_      : Use shadow mapping.
// _VISIBILITY_MAPS_     : Uses visibility maps for the shadow mapping
// _BRDF_FUNCTION_       : Name of the BRDF function to use (e.g. "blinn_phong")
// _MARK_PIXELS_DONE_    : Marks the "pixels_done" texture if the rendered fragment
//                         is opaque (alpha == 1)
// _TEST_PIXELS_DONE_    : Discards the fragment if the corresponding fragment
//                         in the "pixels_done" texture is marked

#version 330 core

// ---------------------------------------------------------------------
// Check for incompatibilities
#if (defined _VISIBILITY_MAPS_) && (!defined _SHADOW_MAPPING_)
	#error "_VISIBILITY_MAPS_ is defined but _SHADOW_MAPPING_ is not!"
#endif

// ---------------------------------------------------------------------
precision highp float;
precision highp int;

// Attributes:
in vec2 vertex_position;
in vec2 vertex_texcoords;

// Varyings:
smooth out vec2 var_texcoords;

// main:
void main()
{
	var_texcoords = vertex_texcoords;

	gl_Position = vec4(vertex_position, 0.0, 1.0);
}
