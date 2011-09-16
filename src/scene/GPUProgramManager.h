// GPUProgramManager.h

#ifndef GPU_PROGRAM_MANAGER_H
#define GPU_PROGRAM_MANAGER_H

#include "../glutil/GPUProgram.h"
#include <list>
#include "../utils/List.h"

class GPUProgramManager;

GPUProgramManager& getProgramManager();

// ---------------------------------------------------------------------
class GPUProgramRef
{
private:
	glutil::GPUProgram* program;
public:
	GPUProgramRef();
	GPUProgramRef(glutil::GPUProgram* program);
	GPUProgramRef(const GPUProgramRef& ref);
	~GPUProgramRef();

	inline glutil::GPUProgram* ptr() {return program;}
	inline const glutil::GPUProgram* ptr() const {return program;}
	inline glutil::GPUProgram* operator->() {return program;}

	GPUProgramRef& operator=(glutil::GPUProgram* ptr);
	GPUProgramRef& operator=(const GPUProgramRef& ref);

	inline bool operator==(const glutil::GPUProgram* ptr) {return (ptr == this->program);}
	inline bool operator==(const GPUProgramRef& ref) {return (ref.program == this->program);}
};

// ---------------------------------------------------------------------
// This structure represents a program's identifier.
// We consider 2 programs to be equivalent if they have equivalent GPUProgramIDs,
// that is to say:
// - vertex_filename is the same
// - fragment_filename is the same
// - preproc_sym is the same (but order is not important)
struct GPUProgramID
{
	std::string vertex_filename;
	std::string fragment_filename;
	Preprocessor::SymbolList preproc_sym;

	GPUProgramID()
	: vertex_filename(""), fragment_filename(""), preproc_sym()
	{
	}

	GPUProgramID(const GPUProgramID& ref)
	{
		*this = ref;
	}

	GPUProgramID(const char* vertex_filename,
				 const char* fragment_filename)
	: vertex_filename(vertex_filename),
	  fragment_filename(fragment_filename),
	  preproc_sym()
	{
	}

	GPUProgramID& operator=(const GPUProgramID& ref)
	{
		this->vertex_filename   = ref.vertex_filename;
		this->fragment_filename = ref.fragment_filename;
		this->preproc_sym       = ref.preproc_sym;
		return *this;
	}
};

// ---------------------------------------------------------------------
class GPUProgramManager
{
private:
	typedef List<glutil::GPUProgram*> ProgramList;
	ProgramList programs;

private:
	GPUProgramManager();
	~GPUProgramManager();

public:
	// Function to get a program by specifying:
	// - the vertex program file name
	// - the fragment program file name
	// - a NULL-terminated list of strings corresponding to the names of the defined
	//   preprocessor symbols for the shaders.
	// This function can either create and compile a new program or return an existing one.
	// Even in case it returns an existing one, this function is not really optimized, so we should
	// avoid using it in the rendering loop.
	// The returned program is COMPILED but NOT LINKED.
//	GPUProgramRef getProgram(const char* vertex_filename, const char* fragment_filename,
//							 const char* symbol_1, ...);

	// Same function, but based on GPUProgramID.
	// if not NULL, *is_new indicates if a new program has been created or not.
	GPUProgramRef getProgram(const GPUProgramID& program_id, bool* is_new=NULL);

private:
	// This function is called by GPUProgramRef when it deletes a managed GPUProgram,
	// so that GPUProgramManager can remove it from its set of pointers.
	void notifyDeletion(glutil::GPUProgram* ptr);

	glutil::GPUProgram* addNewProgram(const std::string& vertex_filename,
									  const std::string& fragment_filename,
									  const Preprocessor::SymbolList& preproc_sym);

	friend GPUProgramManager& getProgramManager();
	friend class GPUProgramRef;
};

#endif // GPU_PROGRAM_MANAGER_H
