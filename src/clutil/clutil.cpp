// clutil.cpp

#include "clutil.h"
#include "../log/Log.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <sstream>
using namespace std;

namespace clutil
{

struct ErrorString
{
	cl_int error;
	const char* error_string;
};

static ErrorString errors[] = {
	{CL_SUCCESS,							"CL_SUCCESS"},
	{CL_DEVICE_NOT_FOUND,					"CL_DEVICE_NOT_FOUND"},
	{CL_DEVICE_NOT_AVAILABLE,				"CL_DEVICE_NOT_AVAILABLE"},
	{CL_COMPILER_NOT_AVAILABLE,				"CL_COMPILER_NOT_AVAILABLE"},
	{CL_MEM_OBJECT_ALLOCATION_FAILURE,		"CL_MEM_OBJECT_ALLOCATION_FAILURE"},
	{CL_OUT_OF_RESOURCES,					"CL_OUT_OF_RESOURCES"},
	{CL_OUT_OF_HOST_MEMORY,					"CL_OUT_OF_HOST_MEMORY"},
	{CL_PROFILING_INFO_NOT_AVAILABLE,		"CL_PROFILING_INFO_NOT_AVAILABLE"},
	{CL_MEM_COPY_OVERLAP,					"CL_MEM_COPY_OVERLAP"},
	{CL_IMAGE_FORMAT_MISMATCH,				"CL_IMAGE_FORMAT_MISMATCH"},
	{CL_IMAGE_FORMAT_NOT_SUPPORTED,			"CL_IMAGE_FORMAT_NOT_SUPPORTED"},
	{CL_BUILD_PROGRAM_FAILURE,				"CL_BUILD_PROGRAM_FAILURE"},
	{CL_MAP_FAILURE,						"CL_MAP_FAILURE"},

	{CL_INVALID_VALUE,						"CL_INVALID_VALUE"},
	{CL_INVALID_DEVICE_TYPE,				"CL_INVALID_DEVICE_TYPE"},
	{CL_INVALID_PLATFORM,					"CL_INVALID_PLATFORM"},
	{CL_INVALID_DEVICE,						"CL_INVALID_DEVICE"},
	{CL_INVALID_CONTEXT,					"CL_INVALID_CONTEXT"},
	{CL_INVALID_QUEUE_PROPERTIES,			"CL_INVALID_QUEUE_PROPERTIES"},
	{CL_INVALID_COMMAND_QUEUE,				"CL_INVALID_COMMAND_QUEUE"},
	{CL_INVALID_HOST_PTR,					"CL_INVALID_HOST_PTR"},
	{CL_INVALID_MEM_OBJECT,					"CL_INVALID_MEM_OBJECT"},
	{CL_INVALID_IMAGE_FORMAT_DESCRIPTOR,	"CL_INVALID_IMAGE_FORMAT_DESCRIPTOR"},
	{CL_INVALID_IMAGE_SIZE,					"CL_INVALID_IMAGE_SIZE"},
	{CL_INVALID_SAMPLER,					"CL_INVALID_SAMPLER"},
	{CL_INVALID_BINARY,						"CL_INVALID_BINARY"},
	{CL_INVALID_BUILD_OPTIONS,				"CL_INVALID_BUILD_OPTIONS"},
	{CL_INVALID_PROGRAM,					"CL_INVALID_PROGRAM"},
	{CL_INVALID_PROGRAM_EXECUTABLE,			"CL_INVALID_PROGRAM_EXECUTABLE"},
	{CL_INVALID_KERNEL_NAME,				"CL_INVALID_KERNEL_NAME"},
	{CL_INVALID_KERNEL_DEFINITION,			"CL_INVALID_KERNEL_DEFINITION"},
	{CL_INVALID_KERNEL,						"CL_INVALID_KERNEL"},
	{CL_INVALID_ARG_INDEX,					"CL_INVALID_ARG_INDEX"},
	{CL_INVALID_ARG_VALUE,					"CL_INVALID_ARG_VALUE"},
	{CL_INVALID_ARG_SIZE,					"CL_INVALID_ARG_SIZE"},
	{CL_INVALID_KERNEL_ARGS,				"CL_INVALID_KERNEL_ARGS"},
	{CL_INVALID_WORK_DIMENSION,				"CL_INVALID_WORK_DIMENSION"},
	{CL_INVALID_WORK_GROUP_SIZE,			"CL_INVALID_WORK_GROUP_SIZE"},
	{CL_INVALID_WORK_ITEM_SIZE,				"CL_INVALID_WORK_ITEM_SIZE"},
	{CL_INVALID_GLOBAL_OFFSET,				"CL_INVALID_GLOBAL_OFFSET"},
	{CL_INVALID_EVENT_WAIT_LIST,			"CL_INVALID_EVENT_WAIT_LIST"},
	{CL_INVALID_EVENT,						"CL_INVALID_EVENT"},
	{CL_INVALID_OPERATION,					"CL_INVALID_OPERATION"},
	{CL_INVALID_GL_OBJECT,					"CL_INVALID_GL_OBJECT"},
	{CL_INVALID_BUFFER_SIZE,				"CL_INVALID_BUFFER_SIZE"},
	{CL_INVALID_MIP_LEVEL,					"CL_INVALID_MIP_LEVEL"},
	{CL_INVALID_GLOBAL_WORK_SIZE,			"CL_INVALID_GLOBAL_WORK_SIZE"},
};

void checkCLErrors(cl_int error, const char* file, unsigned line, const char* function)
{
	static const unsigned nb_errors = sizeof(errors) / sizeof(ErrorString);

#if MAX_CL_ERRORS > 0
	static unsigned nb_reported_errors = 0;

	if(nb_reported_errors < MAX_CL_ERRORS)
	{
#endif
		if(error != CL_SUCCESS)
		{
			string error_string = "";
			for(unsigned i=0 ; i < nb_errors ; i++)
			{
				if(errors[i].error == error)
				{
					error_string = errors[i].error_string;
					break;
				}
			}

			if(error_string.empty())
			{
				stringstream ss;
				ss << "unknown error (code value : 0x" << hex << int(error) << ")" << flush;
				error_string = ss.str();
			}

			Log::write(LOG_ERROR, file, line, function, "OpenCL error: ", error_string);

#if MAX_CL_ERRORS > 0
			nb_reported_errors++;
			if(nb_reported_errors == MAX_CL_ERRORS)
				logWarn("[reached limit of ", MAX_CL_ERRORS, " OpenCL errors, stop reporting]");
#endif
		}
#if MAX_CL_ERRORS > 0
	}
#endif
}

// Load a program source file
char* loadProgramSource(const char* filename)
{
	ifstream f(filename);
	if(!f)
	{
		cerr << "impossible to read the file" << filename << endl;
		return NULL;
	}

	int filesize = 0;
	char* buf = NULL;

	// Get the file size
	f.seekg(0, ios::end);
	filesize = f.tellg();

	// Allocate memory for the text and get the file's contents
	buf = new char[filesize+1];
	memset(buf, 0, filesize);
	f.seekg(0, ios::beg);
	f.read(buf, filesize);
	buf[filesize] = '\0';

	f.close();

	return buf;
}

// Create a program from a source file:
bool createAndBuildProgramFromFile(	cl_context context,
									cl_device_id device,
									const char* filename,
									cl_program* program)
{
	cl_int error = 0;

	// Load the source file:
	char* program_source = clutil::loadProgramSource(filename);
	if(program_source == NULL)
		return false;

	// Create the program:
	*program = clCreateProgramWithSource(context, 1, (const char**)&program_source, NULL, &error);

	// Release the memory used for storing the source code:
	delete [] program_source;

	// Check if it has been correctly created:
	CL_ASSERT(error);
	if(error != CL_SUCCESS)
		return false;

	// Build the program:
	error = clBuildProgram(*program, 0, NULL, "", NULL, NULL);

	// Print the build log:
	// - build status:
	cl_build_status build_status = CL_BUILD_NONE;
	error |= clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &build_status, NULL);

	if(build_status == CL_BUILD_NONE)              logError  ("build status: CL_BUILD_NONE");
	else if(build_status == CL_BUILD_ERROR)        logFailed ("build status: CL_BUILD_ERROR");
	else if(build_status == CL_BUILD_SUCCESS)      logSuccess("build status: CL_BUILD_SUCCESS");
	else if(build_status == CL_BUILD_IN_PROGRESS)  logWarn   ("build status: CL_BUILD_IN_PROGRESS");
	else                                           logError  ("build status: unknown value: ", build_status);

	// - build log:
	size_t build_log_size = 0;
	error |= clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &build_log_size);

	cl_char* build_log = new cl_char[build_log_size];
	error |= clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, build_log_size, build_log, NULL);
	cout << "*** OpenCL kernel build log: " << build_log << endl;
	delete [] build_log;

	// Check:
	CL_ASSERT(error);
	if(error != CL_SUCCESS)
		return false;

	return true;
}

}
