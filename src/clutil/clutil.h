// clutil.h

#ifndef CL_UTIL_H
#define CL_UTIL_H

#include <CL/cl.h>
#include <assert.h>

// Maximum number of OpenCL errors that are printed
// by checkCLErrors(). 0 for no limit.
#define MAX_CL_ERRORS 5

#define CL_CHECK(error) clutil::checkCLErrors(error, __FILE__, __LINE__, __FUNCTION__)

#define CL_ASSERT(error)	clutil::checkCLErrors(error, __FILE__, __LINE__, __FUNCTION__);	\
							assert(error == CL_SUCCESS)

namespace clutil
{

// Check OpenCL errors
void checkCLErrors(cl_int error, const char* file="", unsigned line=0, const char* function="");

// Load a program source file
char* loadProgramSource(const char* filename);

// Create a program from a source file:
bool createAndBuildProgramFromFile(	cl_context context,
									cl_device_id device,
									const char* filename,
									cl_program* program);

}

#endif // CL_UTIL_H
