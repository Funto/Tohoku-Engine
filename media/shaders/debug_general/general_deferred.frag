


#version 330 core



precision highp float;
precision mediump int;



#line 0 1







vec4 blinn_phong(	in vec3 light_vec,
					in vec3 view_vec,
					in vec3 normal,
					in vec4 diffuse_value,
					in vec4 specular_value,
					in float specular_exp)
{
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	float n_dot_l = max(dot(normal, light_vec), 0.0);

	
	if(n_dot_l > 0.0)
	{
		
		vec3 diffuse_term = n_dot_l * vec3(diffuse_value);

		
		vec3 half_vector = normalize(light_vec + view_vec);
		float h_dot_n = max(dot(half_vector, normal), 0.0);
		vec3 specular_term = pow(h_dot_n, specular_exp) * vec3(specular_value);

		
		result = vec4(diffuse_term + specular_term, 1.0);
	}

	return result;
}



#line 12 0

#line 0 2










#line 0 3






float pow5(in float x)
{
	float x2 = x*x;
	return x2*x2*x;
}


vec3 computeFresnel(in vec3 F0, in float cos_i)
{
	return F0 + (vec3(1.0) - F0) * pow5(1.0 - cos_i);
}



#line 10 2

vec4 ashikhmin_shirley(	in vec3 light_vec,
						in vec3 view_vec,
						in vec3 normal,
						in vec4 diffuse_value,
						in vec4 specular_value,
						in float specular_exp)
{
	vec3 half_vector = normalize(light_vec + view_vec);

	float n_dot_l = max(dot(normal, light_vec),     0.0);
	float n_dot_v = max(dot(normal, view_vec),      0.0);
	float n_dot_h = max(dot(normal, half_vector),   0.0);
	float v_dot_h = max(dot(view_vec, half_vector), 0.0);

	float PI = 3.14159265358979323846264;
	vec3 one = vec3(1.0);

	
	vec3 diffuse_term =	(28.0/(23.0*PI)) *
						vec3(diffuse_value) *
						(one - vec3(specular_value)) *
						(1.0 - pow5(1.0 - 0.5*n_dot_v)) *
						(1.0 - pow5(1.0 - 0.5*n_dot_l));

	
	float numerator    =	pow(n_dot_h, specular_exp);
	float denominator  =	v_dot_h * max(n_dot_l, n_dot_v);

	vec3 specular_term =	(specular_exp + 1.0)/(8.0*PI) *
							(numerator / denominator) *
							computeFresnel(vec3(diffuse_value), v_dot_h);

	
	return vec4(diffuse_term + specular_term, 1.0);
	
	
}



#line 13 1




	



		uniform vec4 material_diffuse;


	
	uniform vec4 material_specular;
	uniform float material_specular_exp;











#line 39










	smooth in vec3 var_normal;






#line 57



	
	smooth in vec3 var_eye_space_pos;

	









#line 74








	
	out vec4 frag_position;		
	out vec4 frag_normal;		
	out vec4 frag_diffuse;		
	out vec4 frag_specular;		
	









void main()
{
	
	vec3 normal;






		normal = normalize(var_normal);


	

		vec4 diffuse_value;



			diffuse_value = material_diffuse;



	




		
		
		frag_position = vec4(var_eye_space_pos, 0.0);
		frag_normal   = vec4(normal, 0.0);
		frag_diffuse  = vec4(diffuse_value.rgb, 1.0);	
		frag_specular = vec4(material_specular.rgb, material_specular_exp);
		

	




	


			




















			

				



			




















#line 190

}
