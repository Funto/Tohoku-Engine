// Preprocessor.h
// 2010 by Lionel Fuentes.

/*
This is a simple general-purpose preprocessor which has been made with
GLSL's syntax in mind.

It does not support adding custom include directories (though adding
it is not really difficult), nor does it support parsing strings in
the target language (GLSL has no strings).

No check is done for multi-inclusion problems.

One particular feature is the #for directive.
#for directives can NOT be nested.
If the number of iterations is <= 0, nothing is done.
Example:
#for NB_LIGHTS
	frag_color += light_value_@;
#endfor

When inserting a #line, we support the GLSL 3.30+ syntax, i.e.
the number of the line is the number of the NEXT line, not the current one
(this changed in the GLSL specification with version 3.30).

Supported directives:
#include
#define
#undef
#if
#ifdef
#ifndef
#else
#endif
#for
#endfor

Supported operators:
()
defined, +, -, ~, !
*, /, %
+, -
<<, >>
<, >, <=, >=
==, !=
&
^
|
&&
||
*/

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include <list>
#include <vector>
#include <string>
#include <sstream>
#include "../Common.h"

// Configuration: should we keep a separate list of symbols before
// preprocessing has occured?
#define PREPROC_KEEP_ORIGINAL_SYMBOLS

class ExpressionEvaluator;

struct PreprocSym
{
	std::string name;
	std::string value;

	PreprocSym(const char* name="", const std::string& value="")
	: name(name), value(value)
	{
	}

	PreprocSym(const char* name, int number_value)
	: name(name), value("")
	{
		std::stringstream ss;
		ss << number_value << std::flush;
		this->value = ss.str();
	}

	PreprocSym(const PreprocSym& ref)
	{
		*this = ref;
	}

	inline PreprocSym& operator=(const PreprocSym& ref)
	{
		this->name = ref.name;
		this->value = ref.value;
		return *this;
	}

	inline bool operator==(const PreprocSym& ref) const
	{
		return (this->name == ref.name) && (this->value == ref.value);
	}
};

class Preprocessor
{
public:
	typedef std::vector<PreprocSym> SymbolList;
private:
	// Symbols:
	SymbolList symbols;
	ExpressionEvaluator* evaluator;

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	SymbolList original_symbols;
#endif

	// Current directory (for the #include directive).
	// Includes a final '/' or '\\'.
	std::string directory;

	// #line directive support (GLSL)
	bool use_line_directive;
	int source_string_number;

	// Support for the #for directive:
	const char* for_directive_line;	// Pointer to the line where the #for directive is.
									// NULL means we are not in a #for directive.
	uint for_directive_counter;	// Counter for #for.
	uint for_directive_nb_iter;	// Number of iterations
	uint for_directive_num_line;	// Number of the line where the #for directive was

public:
	Preprocessor(bool use_line_directive=true);
	virtual ~Preprocessor();

	// Symbol management functions:
	void setSymbols(const char* symbol0, ...);
	void setSymbols(const SymbolList& symbols);
	void setSymbols(const std::list<std::string>& symbols);
	void addSymbol(const PreprocSym& symbol);

	const SymbolList& getSymbols() const;
	uint getNbSymbols() const;

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	const SymbolList& getOriginalSymbols() const;
	uint getNbOriginalSymbols() const;
#endif

	bool hasSymbol(const std::string& symbol_name) const;
	PreprocSym* getSymbolByName(const std::string& symbol_name);
	const PreprocSym* getSymbolByName(const std::string& symbol_name) const;

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	bool hasOriginalSymbol(const std::string& symbol_name) const;
	PreprocSym* getOriginalSymbolByName(const std::string& symbol_name);
	const PreprocSym* getOriginalSymbolByName(const std::string& symbol_name) const;
#endif

	void clearSymbols();

	// Get a string of the form '"SYMBOL_1=1", "SYMBOL_2=0"' (useful for debugging)
	std::string getSymbolsString() const;

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	std::string getOriginalSymbolsString() const;
#endif

	// Options of the preprocessor management:
	inline void setUseLineDirective(bool use=true) {use_line_directive = use;}
	inline bool usesLineDirective() const {return use_line_directive;}
	inline int sourceStringNumber() const {return source_string_number;}
	inline void resetSourceStringNumber() {source_string_number = 0;}

	// Execute the preprocessor:
	const char* preprocess(const char* original_source, const char* filename);
	const char* preprocessFromFile(const char* filename);

private:
	// Remove comments, set all newlines to '\n', and add a final '\n' if there is none
	const char* removeComments(const char* original_source, const char* filename);

	// Loading a text file to a buffer:
	const char* loadText(const char* filename) const;

	// Retrieving a directory starting from a file name:
	std::string retrieveDirectory(const char* filename) const;

	// Preprocess a line.
	// An error is indicated by nb_generated_lines == 0
	std::string preprocessLine(const char* line,
							   const char* filename,
							   uint* num_line,
							   std::list<bool>* commented_out,
							   uint* nb_generated_lines,
							   const char** new_line_ptr);

	// Replace the symbols in an expression.
	// If in_directive == true, this means replaceSymbols() is called for an expression
	// inside a directive, in which case we do not replace symbols preceded with a "defined"
	// NB: the end of the expression is marked with '\n' or '\0'!
	std::string replaceSymbols(const char* line, bool in_directive);

	bool includeFile(const char* directive, const char* filename, uint num_line, std::string* result, uint* nb_generated_lines);
	bool defineSymbol(const char* directive);
	bool undefSymbol(const char* directive);
	bool condExpression(const char* directive, bool* is_true);	// for #if and #elif
	bool definedExpression(const char* directive, bool* is_defined);	// for #ifdef and #ifndef
	bool forExpression(const char* directive, const char* filename, uint num_line);
	bool endFor(const char* directive, const char** new_line_ptr, std::string* result, uint* next_num_line);
};

#endif // PREPROCESSOR_H
