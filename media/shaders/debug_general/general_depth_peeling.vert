


















#version 330 core



precision highp float;
precision highp int;




uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform mat4 projection_matrix;





#line 38











#line 50





in vec3 vertex_position;



	in vec3 vertex_normal;










	smooth out vec3 var_normal;






#line 78


















#line 97




void main()
{
	
	mat4 modelview_matrix = view_matrix * model_matrix;

	

	var_normal = mat3(modelview_matrix) * vertex_normal;


	
	vec4 eye_space_pos = modelview_matrix * vec4(vertex_position, 1.0);

	




	
	




#line 126


	






	



#line 140


	
	gl_Position = projection_matrix * eye_space_pos;
}
