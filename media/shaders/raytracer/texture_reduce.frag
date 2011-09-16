// raytracer/texture_reduce.frag

#version 330 core

precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
// Uniforms:
#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	uniform sampler2DRect tex_debug;
#endif
uniform sampler2DRect tex_position;
uniform sampler2DRect tex_power;
uniform sampler2DRect tex_coming_dir;

uniform int offset_x;

// ---------------------------------------------------------------------
// Fragment shader output:
#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	out vec4 frag_debug;
#endif
out vec4 frag_position;
out vec4 frag_power;
out vec4 frag_coming_dir;

// ---------------------------------------------------------------------
void main()
{
	vec2 texcoords = vec2(0.0, gl_FragCoord.y);
	for(int i=offset_x ; i <= _TARGET_WIDTH_-offset_x ; i++)
	{
		texcoords.x = float(i);
		vec4 power = texture(tex_power, texcoords);

		if(dot(power.rgb, power.rgb) > 0.1)
		{
			#ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
				frag_debug = texture(tex_debug, texcoords);
			#endif
			frag_position = texture(tex_position, texcoords);
			frag_power = power;
			frag_coming_dir = texture(tex_coming_dir, texcoords);
			break;
		}
	}

//~ #ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	//~ frag_debug    = texture(tex_debug, gl_FragCoord.xy);
//~ #endif
	//~ frag_position = texture(tex_power, gl_FragCoord.xy);
	//~ frag_power    = texture(tex_position, gl_FragCoord.xy);

//~ #ifdef _DEBUG_USE_DEBUG_FBO_ATTACHMENT_
	//~ frag_debug    = vec4(1.0, 0.0, 0.0, 0.0);
//~ #endif
	//~ frag_position = vec4(0.0, 1.0, 0.0, 0.0);
	//~ frag_power    = vec4(0.0, 0.0, 1.0, 0.0);
}
