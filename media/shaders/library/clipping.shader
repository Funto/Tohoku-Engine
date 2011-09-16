// clipping.shader

#ifndef __CLIPPING_SHADER__
#define __CLIPPING_SHADER__

// ---------------------------------------------------------------------
// Clipping: p0 and p1 are both in clip space
// TODO: near plane clipping seems buggy, far plane clipping is also maybe.
// This tries to implement the first version of the algorithm presented by
// Jim Blinn in "Clipping using Homogeneous Coordinates" (SIGGRAPH '78),
// i.e. the one which considers w > 0 (I'm not sure we can use this algorithm here...)
// TODO: use GLSL's mix() function
void lineClip(inout vec4 p0, inout vec4 p1)
{
	float a;

	float bound_left_0   = p0.w + p0.x;
	float bound_right_0  = p0.w - p0.x;
	float bound_bottom_0 = p0.w + p0.y;
	float bound_top_0    = p0.w - p0.y;
	float bound_near_0   = p0.z;
	float bound_far_0    = p0.w - p0.z;

	float bound_left_1   = p1.w + p1.x;
	float bound_right_1  = p1.w - p1.x;
	float bound_bottom_1 = p1.w + p1.y;
	float bound_top_1    = p1.w - p1.y;
	float bound_near_1   = p1.w + p1.z;
	float bound_far_1    = p1.w - p1.z;

	vec4 prev_p0 = p0;

	// Clip p0:
	a = 0.0;

	if(bound_left_1 < 0.0)
		a = max(a, bound_left_0 / (bound_left_0 - bound_left_1));

	if(bound_right_1 < 0.0)
		a = max(a, bound_right_0 / (bound_right_0 - bound_right_1));

	if(bound_bottom_1 < 0.0)
		a = max(a, bound_bottom_0 / (bound_bottom_0 - bound_bottom_1));

	if(bound_top_1 < 0.0)
		a = max(a, bound_top_0 / (bound_top_0 - bound_top_1));

	if(bound_near_1 < 0.0)
		a = max(a, bound_near_0 / (bound_near_0 - bound_near_1));

	if(bound_far_1 < 0.0)
		a = max(a, bound_far_0 / (bound_far_0 - bound_far_1));

	a = min(a, 1.0);

	p0 = (1.0-a)*p0 + a*p1;

	// Clip p1:
	a = 1.0;

	if(bound_left_1 < 0.0)
		a = min(a, bound_left_0 / (bound_left_0 - bound_left_1));

	if(bound_right_1 < 0.0)
		a = min(a, bound_right_0 / (bound_right_0 - bound_right_1));

	if(bound_bottom_1 < 0.0)
		a = min(a, bound_bottom_0 / (bound_bottom_0 - bound_bottom_1));

	if(bound_top_1 < 0.0)
		a = min(a, bound_top_0 / (bound_top_0 - bound_top_1));

	if(bound_near_1 < 0.0)
		a = min(a, bound_near_0 / (bound_near_0 - bound_near_1));

	if(bound_far_1 < 0.0)
		a = min(a, bound_far_0 / (bound_far_0 - bound_far_1));

	a = max(a, 0.0);

	p1 = (1.0-a)*prev_p0 + a*p1;
}

#endif // __CLIPPING_SHADER__
