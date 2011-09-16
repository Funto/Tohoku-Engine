


















#version 330 core



precision highp float;
precision highp int;




uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;




		uniform vec3 light_pos_0;
#line 38











#line 50





in vec3 vertex_position;



	in vec3 vertex_normal;










	smooth out vec3 var_normal;





		smooth out vec3 var_light_vec_0;
#line 78




	smooth out vec3 var_eye_space_pos;













#line 97




void main()
{
	
	mat4 modelview_matrix = view_matrix * model_matrix;

	

	var_normal = mat3(modelview_matrix) * vertex_normal;


	
	vec4 eye_space_pos = modelview_matrix * vec4(vertex_position, 1.0);

	

	var_eye_space_pos = vec3(eye_space_pos);


	
	


		vec3 eye_space_light_pos_0 = vec3(view_matrix * vec4(light_pos_0, 1.0));
		var_light_vec_0 = eye_space_light_pos_0 - eye_space_pos.xyz;
#line 126


	






	



#line 140


	
	gl_Position = projection_matrix * eye_space_pos;
}
