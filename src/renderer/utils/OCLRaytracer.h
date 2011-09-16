// OCLRaytracer.h

#ifndef OCL_RAYTRACER_H
#define OCL_RAYTRACER_H

#include "../../Common.h"
#include "../../glutil/glutil.h"
#include "../../clutil/clutil.h"

class BounceMap;
class GBuffer;

class OCLRaytracer
{
private:
	// Dimensions of the depth layers:
	uint width;
	uint height;

	// OpenCL context, device and command queue
	cl_context         context;
	cl_device_id       device;
	cl_command_queue   queue;

	// OpenCL program and kernels:
	cl_program         program;
	cl_kernel          raytrace_kernel;

	// Intersection map:
	// TODO: have several of those...
	GLuint id_intersection_map;
	cl_mem intersection_map_mem;

public:
	OCLRaytracer(uint width, uint height);
	virtual ~OCLRaytracer();

	void setup();
	void cleanup();

	void run(BounceMap* bounce_map, GBuffer** gbuffers, uint nb_gbuffers);

	// Getters:
	uint getWidth()  const {return width;}
	uint getHeight() const {return height;}

	GLuint getIntersectionMap() const {return id_intersection_map;}

private:

};

#endif // OCL_RAYTRACER_H
