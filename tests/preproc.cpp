// preproc.cpp

#include "../src/glutil/GPUProgram.h"
#include <iostream>
using namespace std;

namespace glutil
{

void gpuprogram_unittest()
{
/*	GPUProgram* p = new GPUProgram();	// We do not free the memory because ~GPUProgram()
										// assumes GPUProgram(const char*, const char*) has
										// been called, an OpenGL context is created...etc.

	p->setPreprocessorSymbols("THIS_IS_DEFINED", "ALSO_DEFINED", NULL);

	const char* src = p->loadText("program.vert");
	cout << "src == \"" << src << "\"" << endl;

	uint nb_lines = 0;
	const char** preproc = p->preprocess("program.vert", src, &nb_lines);

	delete [] src;

	for(uint i=0 ; i < nb_lines ; i++)
	{
		cout << "preproc[" << i << "] == \"" << preproc[i] << "\"" << endl;
	}

	for(uint i=0 ; i < nb_lines ; i++)
		delete preproc[i];

	delete [] preproc;
*/
}

}

int main()
{
	glutil::gpuprogram_unittest();
	return 0;
}
