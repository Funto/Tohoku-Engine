// bounce_map.frag
// TODO: add transmissive
// TODO: frag_output1.a and invCos_fov
// TODO: compute the size of the bounce map

// Improvements/changes:
// - light space calculations -> cos_L == light_vec.z
// - not scaled by invCos_fov
// - no do/while() in the shader (cosPowHemiRandom() instead)
// - no separation between glossy specular and mirror, scaling the path density
//   and using cosPowHemiRandom() instead

#version 330 core

// ---------------------------------------------------------------------
// Default precision
precision highp float;
precision mediump int;

// ---------------------------------------------------------------------
#include "library/utils.shader"
#include "library/random.shader"

#define TRANSMITTANCE 0.0	// TODO!
#define SPECULAR_FACTOR 10.0	// Used for computing the output vector in case of
							// specular reflections
#define SPECULAR_FACTOR_PATH_DENSITY 0.3	// Used for computing the path
											// density in case of specular reflections
											// NB: the path density should not be above 1.0,
											// as it is used as the last argument argument
											// of the GLSL mix() interpolation function
											// in photon_volume.vert

// ---------------------------------------------------------------------
// Uniforms:
uniform sampler2DRect tex_positions;
uniform sampler2DRect tex_normals;
uniform sampler2DRect tex_diffuse;
uniform sampler2DRect tex_specular;

uniform vec3 light_color;
uniform vec3 light_pos_ws;

uniform float seed_offset;

// Fragment shader output:
out vec4 frag_output0;
out vec4 frag_output1;

// ---------------------------------------------------------------------
void scatter_REFRACT(in float  r,					// random, in the range [0.0, 1.0]
			 in vec3   normal,				// normal
			 in vec3   w_i,					// incident direction
			 in vec3   power_i,				// incident power (color)
			 in vec3   rho_L,				// probability of a lambertian scattering
			 in vec3   rho_S,				// probability of a specular scattering
			 in vec3   rho_T,				// probability of a transmissive scattering
			 in float  specular_exp_value,	// specular exponent, in [0, 1] (TODO)
			 out vec3  w_o,					// output direction
			 out vec3  power_o,				// output power (color)
			 out float path_density)		// path density estimation
{
	// Lambertian scattering:
/*	float rho_L_mean = mean(rho_L);
	r -= rho_L_mean;
	if(r <= 0.0)
	{
		// We multiply power_i by (rho_L / rho_L_mean) so that
		// mean(power_o) == mean(power_i)
		// (because mean(rho_L / rho_L_mean) == 1).
		// We can't apply Russian roulette sampling independently to the 3 colors,
		// so we change the distribution of colors in the power of the photon,
		// while keeping the photon's energy to its original value.
		power_o = power_i * rho_L / rho_L_mean;

		// NB: the output direction is independant on the position of the light
		// because it's a lambertian scattering
		w_o = cosHemiRandom(normal);

		// Low path density because it scattered very diffusely:
		// the path has a very low probability of being taken (compared to specular)
		// -> results in big kernel widths later on.
		path_density = rho_L_mean * 0.01;
		return;
	}

	// Specular scattering:
	float rho_S_mean = mean(rho_S);
	r -= rho_S_mean;
	if(r <= 0.0)
	{
		power_o = power_i * rho_S / rho_S_mean;

		// The main part of the specular component of the
		// BRDF is pow(dot(w_h, n), specular_exp_value),
		// so we choose a vector w_h around the normal according
		// to a "cos(theta)^spec_exp" distribution
		// and recompute the output vector w_o from this.
		vec3 w_h = cosPowHemiRandom(normal, SPECULAR_FACTOR*specular_exp_value);

		// GLSL's reflect() function follows the opposite of our convention
		// (mirrors around the surface indicated by the normal instead of
		// mirroring around the normal itself).
		w_o = reflect(-w_i, w_h);

		// Path density depends on the value of the specular exponent
		// and on the power of the photon.
		path_density = min(1.0, SPECULAR_FACTOR_PATH_DENSITY * specular_exp_value * rho_S_mean);
		return;
	}
*/
	// Transmissive scattering:
	float rho_T_mean = mean(rho_T);
	//r -= rho_T_mean;
	// Must be the case that this was transmissive:
	{
		// TODO: handle transmissive scattering
		// (for the moment: absorbed)
		power_o = vec3(0.0);
		w_o = vec3(0.0);
		path_density = 0.0;

		power_o = power_i;
		w_o = refract(-w_i, normal, 1.0/1.8);
		path_density = 1.0;

		//~ power_o = power_i * rho_T / rho_T_mean;
		//~ w_o = refract(-w_i, normal, eta_i / eta_material);
//~
		//~ // w_o is zero on total internal refraction
		//~ if(dot(w_o, w_o) == 0.0)
			//~ discard;
		//~ else
		//~ {
			//~ path_density = p_T_mean;
			//~ return;
		//~ }
	}
}

// ---------------------------------------------------------------------
void scatter(in float  r,					// random, in the range [0.0, 1.0]
			 in vec3   normal,				// normal
			 in vec3   w_i,					// incident direction
			 in vec3   power_i,				// incident power (color)
			 in vec3   rho_L,				// probability of a lambertian scattering
			 in vec3   rho_S,				// probability of a specular scattering
			 in vec3   rho_T,				// probability of a transmissive scattering
			 in float  specular_exp_value,	// specular exponent, in [0, 1] (TODO)
			 out vec3  w_o,					// output direction
			 out vec3  power_o,				// output power (color)
			 out float path_density)		// path density estimation
{
	// Lambertian scattering:
	float rho_L_mean = mean(rho_L);
	r -= rho_L_mean;
	if(r <= 0.0)
	{
		// We multiply power_i by (rho_L / rho_L_mean) so that
		// mean(power_o) == mean(power_i)
		// (because mean(rho_L / rho_L_mean) == 1).
		// We can't apply Russian roulette sampling independently to the 3 colors,
		// so we change the distribution of colors in the power of the photon,
		// while keeping the photon's energy to its original value.
		power_o = power_i * rho_L / rho_L_mean;

		// NB: the output direction is independant on the position of the light
		// because it's a lambertian scattering
		w_o = cosHemiRandom(normal);

		// Low path density because it scattered very diffusely:
		// the path has a very low probability of being taken (compared to specular)
		// -> results in big kernel widths later on.
		path_density = rho_L_mean * 0.01;
		return;
	}


	// Specular scattering:
	float rho_S_mean = mean(rho_S);
/*	r -= rho_S_mean;
	if(r <= 0.0)
	{
*/		power_o = power_i * rho_S / rho_S_mean;

		// The main part of the specular component of the
		// BRDF is pow(dot(w_h, n), specular_exp_value),
		// so we choose a vector w_h around the normal according
		// to a "cos(theta)^spec_exp" distribution
		// and recompute the output vector w_o from this.
		vec3 w_h = cosPowHemiRandom(normal, SPECULAR_FACTOR*specular_exp_value);

		// GLSL's reflect() function follows the opposite of our convention
		// (mirrors around the surface indicated by the normal instead of
		// mirroring around the normal itself).
		w_o = reflect(-w_i, w_h);

		// Path density depends on the value of the specular exponent
		// and on the power of the photon.
		path_density = min(1.0, SPECULAR_FACTOR_PATH_DENSITY * specular_exp_value * rho_S_mean);
		return;
/*	}
*/
/*	// Transmissive scattering:
	float rho_T_mean = mean(rho_T);
	//r -= rho_T_mean;
	// Must be the case that this was transmissive:
	{
		// TODO: handle transmissive scattering
		// (for the moment: absorbed)
		power_o = vec3(0.0);
		w_o = vec3(0.0);
		path_density = 0.0;

		power_o = power_i;
		//w_o = refract(-w_i, normal, 1.0);
		* w_o = refract(-w_i, normal, 1.0 / 1.8);
		path_density = 1.0;

		//~ power_o = power_i * rho_T / rho_T_mean;
		//~ //w_o = refract(-w_i, normal, eta_i / eta_material);
//~
		//~ // w_o is zero on total internal refraction
		//~ if(dot(w_o, w_o) == 0.0)
			//~ discard;
		//~ else
		//~ {
			//~ path_density = p_T_mean;
			//~ return;
		//~ }
	}
*/
}

// ---------------------------------------------------------------------
// main:
void main()
{
	// Read the normal:
	vec4 texel_normal = texture(tex_normals,   gl_FragCoord.xy);
	vec3 normal       = texel_normal.xyz;		// light-space normal

	// Check if there is nothing (normal is 0):
	if(dot(normal, normal) < 0.5)
		discard;

	// Initialize the random numbers generator with a hash of
	// the pixel position and light position in world space:
	/*vec2 seed = gl_FragCoord.xy +
				vec2(light_pos_ws.x + light_pos_ws.z,
				     light_pos_ws.y + light_pos_ws.z);
	*/
	vec2 seed = gl_FragCoord.xy;

	seed += vec2(seed_offset);
	initRand(seed);

	// Texture reads:
	vec4 texel_position = texture(tex_positions, gl_FragCoord.xy);
	vec4 texel_diffuse  = texture(tex_diffuse,   gl_FragCoord.xy);
	vec4 texel_specular = texture(tex_specular,  gl_FragCoord.xy);

	// Get values from the read texels:
	vec3 position           = texel_position.xyz;	// light-space position
	vec4 diffuse_value      = texel_diffuse;		// diffuse value
	vec4 specular_value     = texel_specular;		// specular value

	vec3 light_vec          = -position;			// light-space light vector: pos -> light
	float dist_to_light     = unitize(light_vec);

	// Cosine of the angle of incidence:
	float cos_i = max(0.0, dot(light_vec, normal));

	// Compute Fresnel reflection coefficient:
	vec3 F0 = specular_value.rgb;
	vec3 F  = computeFresnel(F0, cos_i);

	// Compute the probabilities of scattering.
	// probability of absorption = 1.0 - rho_L - rho_S - rho_T
	vec3 rho_L = diffuse_value.rgb;
	vec3 rho_S = F;
	vec3 rho_T = TRANSMITTANCE * (vec3(1.0) - F);
	vec3 rho_total = rho_L + rho_S + rho_T;

	// Random variable that decides what kind of scattering occurs:
	float r = rand1();

	// Test if the photon is absorbed.
	// We multiply by 3.0 (number of components: R, G, B) to avoid dividing
	// the right hand side:
	//~ if(r * 3.0 >= sum(rho_total))
		//~ discard;

	// Cosine of the angle between -look_vector and the light vector (w_i == light_vec).
	// The look vector is the vector which points to the negative Z direction
	// in light space (i.e where the light is looking at).
	// As we are already in light space, this is just vec3(0.0, 0.0, -1.0),
	// so the dot product gets simplified to:
	float cos_look = light_vec.z;

	// Incident power.
	// The light sends the same amount of energy in each direction.
	// Because of Lambert's law, more energy goes through the pixel in the middle
	// of the light plane than to the pixels in the borders, and this follows
	// a cosine law. This is because the pixel's solid angle from the light position
	// gets smaller when we are in the borders of the bounce map because of the
	// projection.
	// For the pixel in the middle: cos_look == 1.0 => unchanged
	// For the pixel in the corner: cos_look < 1.0  => more power is given
	// This power needs to be divided afterwards by the number of photons (number of
	// pixels in the bounce map), as here we consider the whole energy of the light.
	vec3 power_i = light_color / cos_look;

	// Output of the scatter() function:
	vec3 w_o = vec3(0.0);		// Eye-space outgoing vector
	vec3 power_o = vec3(0.0);	// Outgoing radiance, scaled by the number of photons (like power_i)
	float path_density = 1.0;

	// Scatter!
	scatter(r,
			normal,
			light_vec,	// w_i
			power_i,
			rho_L,
			rho_S,
			rho_T,
			specular_value.a,	// specular_exp_value
			w_o,
			power_o,
			path_density);

	// Write the output:
	frag_output0.rgb = power_o;
	frag_output0.a = 0.0;

	frag_output1.rgb = w_o;
	frag_output1.a = path_density;

	// ---------------------------------------------------------------------
	// BEGIN DEBUG
	//~ frag_output0.rgb = light_vec;
	//~ frag_output0.rgb = rho_S;
	//~ frag_output0.rgb = vec3(1.0, 0.0, 0.0);
	//~ frag_output0.rgb = F;
	//~ frag_output0.rgb = F0;
	//~ frag_output0.rgb = cos_i * vec3(1.0);
	//~ frag_output0.rgb = pow5(1.0-cos_i) * vec3(1.0);

	/*// Cosine of the angle of incidence:
	float cos_i = max(0.0, dot(light_vec, normal));

	// Compute Fresnel reflection coefficient:
	vec3 F0 = specular_value.rgb;
	vec3 F  = computeFresnel(F0, cos_i);*/



	/*vec3 rho_L = diffuse_value.rgb;
	vec3 rho_S = F;
	vec3 rho_T = TRANSMITTANCE * (vec3(1.0) - F);
	vec3 rho_total = rho_L + rho_S + rho_T;*/

	// END DEBUG
	// ---------------------------------------------------------------------
}
