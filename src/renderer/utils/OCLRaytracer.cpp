// OCLRaytracer.cpp

#include "../../Boundaries.h"
#include "OCLRaytracer.h"
#include "../../log/Log.h"
#include <CL/cl_gl.h>
#include "GBuffer.h"
#include "BounceMap.h"

#ifdef WIN32
	#include <windows.h>
#else
	#include <GL/glx.h>
#endif

#include <iostream>
using namespace std;

OCLRaytracer::OCLRaytracer(uint width, uint height)
: width(width), height(height)
{
}

OCLRaytracer::~OCLRaytracer()
{
}

void OCLRaytracer::setup()
{
	cl_platform_id    platform;
	cl_uint           nb_platforms = 0;
	cl_int            error = CL_SUCCESS;

	// -------- Platform, device, context, command queue -------
	// Get the platform:
	clGetPlatformIDs(1, &platform, &nb_platforms);
	logInfo("number of OpenCL platforms found: ", nb_platforms);

	// Print the OpenCL version:
	char version_string[40] = "";
	clGetPlatformInfo(platform, CL_PLATFORM_VERSION, sizeof(version_string), version_string, NULL);
	logInfo("CL_PLATFORM_VERSION: ", version_string);

	// Get the GPU device
	CL_ASSERT(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL));

	// Create the context, while enabling sharing with OpenGL with the cl_khr_gl_sharing OpenCL extension:
	cl_context_properties context_properties[] = {
#ifdef WIN32
		CL_GL_CONTEXT_KHR,  (cl_context_properties)(wglGetCurrentContext()),
		CL_WGL_HDC_KHR,     (cl_context_properties)(wglGetCurrentDC()),
#else
		// Assuming X Window System if not Windows:
		CL_GL_CONTEXT_KHR,  (cl_context_properties)(glXGetCurrentContext()),
		CL_GLX_DISPLAY_KHR, (cl_context_properties)(glXGetCurrentDisplay()),
#endif
		0
	};
	context = clCreateContext(context_properties, 1, &device, NULL, NULL, &error);
	CL_ASSERT(error);

	// Create a command queue
	queue = clCreateCommandQueue(context, device, 0, &error);
	CL_ASSERT(error);

	// -------- Program, kernel -------
	// Create and build the program:
	bool ok = clutil::createAndBuildProgramFromFile(context, device, "media/opencl/raytracer.cl", &program);
	assert(ok);

	// Create the kernels:
	raytrace_kernel = clCreateKernel(program, "raytrace", &error);
	CL_ASSERT(error);

	// ------- Memory allocation --------
	id_intersection_map = glutil::createTextureRectRGBAF(width, height, true);
	intersection_map_mem = clCreateFromGLTexture2D(context,
												   CL_MEM_WRITE_ONLY,
												   GL_TEXTURE_RECTANGLE,
												   0,						// mipmap level
												   id_intersection_map,
												   &error);
	CL_ASSERT(error);
}

void OCLRaytracer::cleanup()
{
	// Delete the intersection map:
	clReleaseMemObject(intersection_map_mem);
	glDeleteTextures(1, &id_intersection_map);

	// Release the OpenCL program and kernels:
	clReleaseKernel(raytrace_kernel);
	clReleaseProgram(program);

	// Release the OpenCL context and command queue:
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
}

void OCLRaytracer::run(BounceMap *bounce_map, GBuffer **gbuffers, uint nb_gbuffers)
{
	GLint error = 0;

	// Create OpenCL images from the bounce map:
	cl_mem bounce_map_mem[2];
	for(uint i=0 ; i < 2 ; i++)
	{
		bounce_map_mem[i] = clCreateFromGLTexture2D(context,
													CL_MEM_READ_ONLY,
													GL_TEXTURE_RECTANGLE,
													0,								// mipmap level
													bounce_map->getTexOutput(i),
													&error);
		CL_ASSERT(error);
	}

	// Create OpenCL images from the GBuffers:
	cl_mem gbuffers_mem[NB_MAX_DEPTH_LAYERS];
	for(uint i=0 ; i < nb_gbuffers ; i++)
	{
		/*gbuffers_mem[i] = clCreateFromGLTexture2D(context, CL_MEM_READ_ONLY, GL_TEXTURE_RECTANGLE, 0,
												  gbuffers[i]->getTexDepth(), &error);
												  */
		 gbuffers_mem[i] = clCreateFromGLTexture2D(context,
													CL_MEM_READ_ONLY,
													GL_TEXTURE_RECTANGLE,
													0,								// mipmap level
													bounce_map->getTexOutput(0),
													//gbuffers[i]->getTexDepth(),	// CL_INVALID_VALUE!!
													&error);
		CL_ASSERT(error);
	}

	// Acquire the OpenGL objects:
	// - bounce map:
	error |= clEnqueueAcquireGLObjects(queue,
									   2,	// number of objects
									   bounce_map_mem,	// objects
									   0,	// number of events in wait_list
									   NULL,	// wait_list (events)
									   NULL);	// returned event
	// - intersection map:
	error |= clEnqueueAcquireGLObjects(queue, 1, &intersection_map_mem, 0, NULL, NULL);
	CL_ASSERT(error);

	// Set the arguments for the raytracing kernel:
	cl_uint num_arg = 0;
	error |= clSetKernelArg(raytrace_kernel, num_arg++, sizeof(cl_mem), &bounce_map_mem[0]);
	error |= clSetKernelArg(raytrace_kernel, num_arg++, sizeof(cl_mem), &bounce_map_mem[1]);
	error |= clSetKernelArg(raytrace_kernel, num_arg++, sizeof(cl_mem), &intersection_map_mem);

	for(uint i=0 ; i < nb_gbuffers ; i++)
		error |= clSetKernelArg(raytrace_kernel, num_arg++, sizeof(cl_mem), &gbuffers_mem[i]);

	CL_ASSERT(error);

	// Run the raytracing kernel:
	size_t global_work_size[] = {width, height};
	error |= clEnqueueNDRangeKernel(queue,
									raytrace_kernel,
									2,					// work dimensions
									NULL,				// global work offset: must be NULL
									global_work_size,	// global work size
									NULL,				// local work size
									0, NULL, NULL);	// events stuff
	CL_ASSERT(error);

	// Release the OpenGL objects:
	// - intersection map:
	error |= clEnqueueReleaseGLObjects(queue, 1, &intersection_map_mem, 0, NULL, NULL);

	// - bounce map:
	error |= clEnqueueReleaseGLObjects(queue, 2, bounce_map_mem, 0, NULL, NULL);

	// Release the OpenCL objects:
	for(uint i=0 ; i < 2 ; i++)
		clReleaseMemObject(bounce_map_mem[i]);

	for(uint i=0 ; i < nb_gbuffers ; i++)
		clReleaseMemObject(gbuffers_mem[i]);
}
