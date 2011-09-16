// general.vert
// We assume the modelview matrix is orthogonal.
// Supported symbols:
// _NB_LIGHTS_           : Number of lights to support
// _FORWARD_SHADING_     : Classic forward shading rasterization
// _RENDER_TO_GBUFFER_   : Supports rendering to a G-buffer
// _SHADOW_MAPPING_      : Use shadow mapping.
// _VISIBILITY_MAPS_     : Use visibility maps (has a meaning only if _RENDER_TO_GBUFFER_ is
//                         defined, but it can be defined otherwise, it is just ignored then).
// _BRDF_FUNCTION_       : Name of the BRDF function to use (e.g. "blinn_phong")
// _DEBUG_TEXTURE_       : Adds a debug texture
// TEXTURE_MAPPING       : Supports texture mapping for the diffuse component
// NORMAL_MAPPING        : Supports normal mapping
// DEPTH_PEELING         : Test the fragment against a provided additional depth buffer

// Incompatibilities:
// - _FORWARD_SHADING_ and _RENDER_TO_GBUFFER_ should not be defined at the same time.

#version 330 core

// ---------------------------------------------------------------------
// Check for incompatibilities
#if (defined _FORWARD_SHADING_) && (defined _RENDER_TO_GBUFFER_)
	#error "_FORWARD_SHADING_ and _RENDER_TO_GBUFFER_ are both defined!"
#endif

#if (!defined _FORWARD_SHADING_) && (!defined _RENDER_TO_GBUFFER_)
	#error "neither _FORWARD_SHADING_ nor _RENDER_TO_GBUFFER_ is defined!"
#endif

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision highp int;

// ---------------------------------------------------------------------
// Uniforms
// - transformation matrices
uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;

// - light positions:
#ifdef _FORWARD_SHADING_
	#for _NB_LIGHTS_
		uniform vec3 light_pos_@;
	#endfor
#endif

// - "shadow" matrices
// world space      => light view space		[light view matrix]
// light view space => light clip space		[light proj matrix]
// light clip space => shadow map space		[bias matrix]
// Hence: shadow_matrix = bias_matrix * light_proj_matrix * light_view_matrix
// See RasterRenderer::bindShadowMaps()
#ifdef _SHADOW_MAPPING_
	#for _NB_LIGHTS_
		uniform mat4 shadow_matrix_@;
	#endfor
#endif

// ---------------------------------------------------------------------
// Vertex attributes
// - position
in vec3 vertex_position;

// - normal
#ifndef NORMAL_MAPPING
	in vec3 vertex_normal;
#endif

// - tex coords
#if (defined TEXTURE_MAPPING) || (defined NORMAL_MAPPING)
	in vec2 vertex_texcoords;
#endif

// ---------------------------------------------------------------------
// Varying variables
#ifndef NORMAL_MAPPING
	smooth out vec3 var_normal;
#endif

// - light vectors:
#ifdef _FORWARD_SHADING_
	#for _NB_LIGHTS_
		smooth out vec3 var_light_vec_@;
	#endfor
#endif

// - interpolated position
smooth out vec3 var_eye_space_pos;

// - interpolated texture coordinates
#if (defined TEXTURE_MAPPING) || (defined NORMAL_MAPPING)
	smooth out vec2 var_texcoords;
#endif

// - interpolated texture coordinates for the shadow maps
#ifdef _SHADOW_MAPPING_
	#for _NB_LIGHTS_
		smooth out vec4 var_shadow_texcoords_@;
	#endfor
#endif

// ---------------------------------------------------------------------
// Main function:
void main()
{
	// Compute the modelview matrix:
	mat4 modelview_matrix = view_matrix * model_matrix;

	// Calculate interpolated normal (NB: assuming modelview_matrix is orthogonal!):
	#ifndef NORMAL_MAPPING
		var_normal = mat3(modelview_matrix) * vertex_normal;
	#endif

	// Calculate world position:
	vec4 eye_space_pos = modelview_matrix * vec4(vertex_position, 1.0);

	// Calculate interpolated position
	var_eye_space_pos = vec3(eye_space_pos);

	// Calculate interpolated light vectors:
	// (they are normalized later, in the fragment shader)
	#ifdef _FORWARD_SHADING_
		#for _NB_LIGHTS_
			vec3 eye_space_light_pos_@ = vec3(view_matrix * vec4(light_pos_@, 1.0));
			var_light_vec_@ = eye_space_light_pos_@ - eye_space_pos.xyz;
		#endfor
	#endif

	// Interpolate texture coordinates
	#if (defined TEXTURE_MAPPING) || (defined NORMAL_MAPPING)
		var_texcoords = vertex_texcoords;
	#endif

	// Calculate the texture coordinates we use for fetching from the shadow maps:
	#ifdef _SHADOW_MAPPING_
		#for _NB_LIGHTS_
			var_shadow_texcoords_@ = shadow_matrix_@ * model_matrix * vec4(vertex_position, 1.0);
		#endfor
	#endif

	// Calculate the projected position:
	gl_Position = projection_matrix * eye_space_pos;
}
