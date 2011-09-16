// depth_peeling.frag
// See depth_peeling.vert for explanations about supported symbols.

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
// Uniforms
// - previous depth layer:
uniform sampler2DRectShadow tex_prev_depth_layer;
//~ uniform sampler2DRect tex_prev_depth_layer;

// - normal texture for normal mapping:
#ifdef NORMAL_MAPPING
	uniform sampler2D tex_normal;
#endif

// ---------------------------------------------------------------------
// Varying variables
// - normals:
#ifndef NORMAL_MAPPING
	smooth in vec3 var_normal;
#endif

// - interpolated texture coordinates
#ifdef NORMAL_MAPPING
	smooth in vec2 var_texcoords;
#endif

// ---------------------------------------------------------------------
// Fragment shader output
out vec4 frag_normal;

// ---------------------------------------------------------------------
void main()
{
	// --------- Compare to the previous depth layer ----------
	float keep_fragment = texture(tex_prev_depth_layer, gl_FragCoord.xyz);

	if(keep_fragment == 0.0)
		discard;

	// --------- Calculate normal ----------
	vec3 normal;
	#ifdef NORMAL_MAPPING
		// TODO: this is WRONG, we do not convert from tangent space
		// to eye space
		//(see http://hacksoflife.blogspot.com/2009/11/per-pixel-tangent-space-normal-mapping.html)
		normal = 2.0*texture(tex_normal, var_texcoords).rgb - vec3(1.0);
	#else
		normal = normalize(var_normal);
	#endif

	frag_normal = vec4(normal, 0.0);
}
