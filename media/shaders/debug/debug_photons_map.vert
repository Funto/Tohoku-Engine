// debug/debug_photons_map.vert

#version 330 core

precision highp float;
precision highp int;

uniform sampler2DRect tex_photons_map_0;
uniform sampler2DRect tex_photons_map_1;
uniform mat4 eye_proj_matrix;

uniform int photons_map_size;

in vec2 vertex_position;

out vec3 var_color;

void main()
{
	ivec2 texcoords;
	texcoords.y = gl_InstanceID/photons_map_size;
	texcoords.x = gl_InstanceID - texcoords.y * photons_map_size;

	if(vertex_position.x < 0.5)
	{
		vec4 texel_photons_map_0 = texture(tex_photons_map_0, texcoords.xy);
		vec4 eye_space_pos = vec4(texel_photons_map_0.rgb, 1.0);
		gl_Position = eye_proj_matrix * eye_space_pos;
		
		var_color = eye_space_pos.xyz;
	}
	else
	{
		vec4 texel_photons_map_1 = texture(tex_photons_map_1, texcoords.xy);
		vec4 eye_space_pos = vec4(texel_photons_map_1.rgb, 1.0);
		gl_Position = eye_proj_matrix * eye_space_pos;
		
		var_color = eye_space_pos.xyz;
		//var_color = vec3(1,0,0);
	}
// TODO!

	//var_color = vec3(1,0,1);
	
	//gl_Position = vec4(vertex_position, 0.0, 1.0);
	
	//gl_Position.x += 0.01*gl_InstanceID;
}
