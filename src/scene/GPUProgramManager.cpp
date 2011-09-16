// GPUProgramManager.cpp

#include "GPUProgramManager.h"
#include <cassert>
#include <cstdarg>
#include "../utils/StdListManip.h"
#include "../log/Log.h"
using namespace std;

GPUProgramManager& getProgramManager()
{
	static GPUProgramManager manager;
	return manager;
}

// ---------------------------------------------------------------------
// GPUProgramRef implementation:
GPUProgramRef::GPUProgramRef()
: program(NULL)
{
}

GPUProgramRef::GPUProgramRef(glutil::GPUProgram* program)
: program(program)
{
	if(program != NULL)
		program->ref_count++;
}

GPUProgramRef::GPUProgramRef(const GPUProgramRef& ref)
: program(ref.program)
{
	if(program != NULL)
		program->ref_count++;
}

GPUProgramRef::~GPUProgramRef()
{
	if(program != NULL)
	{
		program->ref_count--;
		if(program->ref_count == 0)
		{
			getProgramManager().notifyDeletion(program);
			delete program;
		}
	}
}

GPUProgramRef& GPUProgramRef::operator=(glutil::GPUProgram* ptr)
{
	// Decrement ref_count or delete previous pointer
	if(program != NULL)
	{
		program->ref_count--;
		if(program->ref_count == 0)
		{
			getProgramManager().notifyDeletion(program);
			delete program;
		}
	}
	// Assign and (optionaly) increment ref_count for new pointer
	this->program = ptr;
	if(this->program != NULL)
		this->program->ref_count++;

	return *this;
}

GPUProgramRef& GPUProgramRef::operator=(const GPUProgramRef& ref)
{
	// Decrement ref_count or delete previous pointer
	if(program != NULL)
	{
		program->ref_count--;
		if(program->ref_count == 0)
		{
			getProgramManager().notifyDeletion(this->program);
			delete this->program;
		}
	}
	// Assign and (optionally) increment ref_count for new pointer
	this->program = ref.program;
	if(this->program != NULL)
		this->program->ref_count++;

	return *this;
}

// ---------------------------------------------------------------------
// GPUProgramManager implementation:
GPUProgramManager::GPUProgramManager()
{
}

GPUProgramManager::~GPUProgramManager()
{
	assert(programs.size() == 0);
}

// Function to get a program by specifying:
// - the vertex program file name
// - the fragment program file name
// - a NULL-terminated list of strings corresponding to the names of the defined
//   preprocessor symbols for the shaders.
// This function can either create and compile a new program or return an existing one.
// Even in case it returns an existing one, this function is not really optimized, so we should
// avoid using it in the rendering loop.
// The new program is ready to use.
/*GPUProgramRef GPUProgramManager::getProgram(const char* vertex_filename, const char* fragment_filename,
											const char* symbol_1, ...)
{
	// Create a GPUProgramID and call getProgram() on it.
	// We put all symbols in program_id.preproc_sym, as 2 program IDs equivalence is
	// independent on the symbols being in program_id.preproc_sym or program_id.additional_preproc_sym.
	GPUProgramID program_id;
	program_id.vertex_filename = vertex_filename;
	program_id.fragment_filename = fragment_filename;

	va_list vl;
	va_start(vl, symbol_1);

	const char* symbol = symbol_1;
	while(symbol != NULL)
	{
		program_id.preproc_sym.push_back(string(symbol));
		symbol = va_arg(vl, const char*);
	}
	va_end(vl);

	return getProgram(program_id);
}*/

// Same function, but based on GPUProgramID:
GPUProgramRef GPUProgramManager::getProgram(const GPUProgramID& program_id, bool* is_new)
{
	ProgramList::Iterator it_end = programs.end();

	// Create a joined list of preprocessor symbols, containing program_id.preproc_sym
	// joined with program_id.additional_preproc_sym:
	const Preprocessor::SymbolList& preproc_sym = program_id.preproc_sym;
	Preprocessor::SymbolList::const_iterator it_sym_end = preproc_sym.end();
	uint nb_preproc_sym = preproc_sym.size();

	// For each program in the list:
	for(ProgramList::Iterator it = programs.begin() ;
		it != it_end ;
		it++)
	{
		glutil::GPUProgram* p = *it;

		// If the filenames and the number of preprocessor symbols correspond:
		// (note: what is important is the list of preprocessor symbols before
		// preprocessing occurs, as this list can change during the preprocessing
		// because of "#define" and "#undef" directives).
		if(	p->getVertexFilename()   == program_id.vertex_filename   &&
			p->getFragmentFilename() == program_id.fragment_filename &&
			nb_preproc_sym == p->getPreprocessor()->getNbOriginalSymbols())
		{
			// Let's check the preprocessor symbols themselves:
			bool corresponds = true;
			for(Preprocessor::SymbolList::const_iterator it_sym = preproc_sym.begin() ;
				it_sym != it_sym_end ;
				it_sym++)
			{
				const PreprocSym* sym = p->getPreprocessor()->getOriginalSymbolByName(it_sym->name);

				if(sym == NULL ||
				   !(*sym == *it_sym))
				{
					corresponds = false;
					break;
				}
			}

			// If they all correspond, return this program
			if(corresponds)
			{
				if(is_new != NULL)
					*is_new = false;
				return GPUProgramRef(p);
			}
		}
	}

	// If we reach this, this means that no corresponding program has been found,
	// so we should create a new one, add it to the list (both operations are done by addNewProgram())
	// and return it:
	if(is_new != NULL)
		*is_new = true;
	return GPUProgramRef(addNewProgram(program_id.vertex_filename, program_id.fragment_filename, preproc_sym));
}

glutil::GPUProgram* GPUProgramManager::addNewProgram(const std::string& vertex_filename,
													 const std::string& fragment_filename,
													 const Preprocessor::SymbolList& preproc_sym)
{
	// Create the program:
	glutil::GPUProgram* p = new glutil::GPUProgram(vertex_filename.c_str(),
												   fragment_filename.c_str());

	// Set the preprocessor symbols:
	p->getPreprocessor()->setSymbols(preproc_sym);

	logInfo("preparing program \"", vertex_filename, "\"/\"", fragment_filename, "\" with original symbols: ",
			p->getPreprocessor()->getOriginalSymbolsString());

	bool ok = p->compileAndAttach();
	assert(ok);

	programs.pushBack(p);

	return p;
}

// This function is called by GPUProgramRef when it deletes a managed GPUProgram,
// so that GPUProgramManager can remove it from its set of pointers.
void GPUProgramManager::notifyDeletion(glutil::GPUProgram* ptr)
{
	ProgramList::Iterator it_end = programs.end();
	for(ProgramList::Iterator it = programs.begin() ;
		it != it_end ;
		it++)
	{
		if(*it == ptr)
		{
			programs.erase(it);
			return;
		}
	}

	assert(false && "notified deletion of a program not in the list !");
}
