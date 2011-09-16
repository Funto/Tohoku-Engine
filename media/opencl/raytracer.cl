// raytracer.cl

const sampler_t sampler_bm =	CLK_NORMALIZED_COORDS_FALSE |
							CLK_ADDRESS_CLAMP_TO_EDGE |
							CLK_FILTER_NEAREST;

__kernel void
raytrace(	__read_only image2d_t bounce_map_0,
		__read_only image2d_t bounce_map_1,
		__write_only image2d_t intersection_map,
		__read_only image2d_t depth_buffer_0,
		__read_only image2d_t depth_buffer_1,
		__read_only image2d_t depth_buffer_2,
		__read_only image2d_t depth_buffer_3,
		__read_only image2d_t depth_buffer_4)
{
	int2 coord = (int2)(get_global_id(0), get_global_id(1));
	
	float4 texel_bm_0 = read_imagef(bounce_map_0, sampler_bm, coord);
	float4 texel_bm_1 = read_imagef(bounce_map_1, sampler_bm, coord);
	
//	float4 result = texel_bm_0 + texel_bm_1;
	float4 result = texel_bm_0;
	write_imagef(intersection_map, coord, result);
}
