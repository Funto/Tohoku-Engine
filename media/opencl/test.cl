// raytracer.cl

kernel void
add(global float *a,
	global float *b,
	global float *answer)
{
	int gid = get_global_id(0);
	answer[gid] = a[gid] + b[gid];
}
