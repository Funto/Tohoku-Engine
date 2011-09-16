// Preprocessor.cpp

#include "Preprocessor.h"
#include "PreprocHelpers.h"
#include "ExpressionEvaluator.h"
#include "../log/Log.h"
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <fstream>
#include <cassert>
#include <sstream>
using namespace std;

//#define DEBUG_PREPROCESSOR

Preprocessor::Preprocessor(bool use_line_directive)
: symbols(),
  evaluator(NULL),
#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
  original_symbols(),
#endif
  use_line_directive(use_line_directive),
  source_string_number(0),
  for_directive_line(NULL),
  for_directive_counter(0),
  for_directive_num_line(1)
{
	evaluator = new ExpressionEvaluator(this);
}

Preprocessor::~Preprocessor()
{
	delete evaluator;
}

// Symbol management functions:
void Preprocessor::setSymbols(const char* symbol0, ...)
{
	this->symbols.clear();
#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	this->original_symbols.clear();
#endif

	const char* symbol = symbol0;
	va_list vl;
	va_start(vl, symbol0);
	while(symbol != NULL)
	{
		this->symbols.push_back(symbol);
#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
		this->original_symbols.push_back(symbol);
#endif
		symbol = va_arg(vl, const char*);
	}
	va_end(vl);
}

void Preprocessor::setSymbols(const SymbolList& symbols)
{
	uint nb_symbols = symbols.size();
	this->symbols.resize(nb_symbols);

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	this->original_symbols.resize(nb_symbols);
#endif

	for(uint i=0 ; i < nb_symbols ; i++)
	{
		this->symbols[i] = symbols[i];
#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
		this->original_symbols[i] = symbols[i];
#endif
	}
}

void Preprocessor::setSymbols(const list<string>& symbols)
{
	uint nb_symbols = symbols.size();
	this->symbols.resize(nb_symbols);

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	this->original_symbols.resize(nb_symbols);
#endif

	uint i=0;
	list<string>::const_iterator it_end = symbols.end();

	for(list<string>::const_iterator it = symbols.begin() ;
		it != it_end ;
		it++, i++)
	{
		this->symbols[i].name = (*it);
		this->symbols[i].value = "";

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
		this->original_symbols[i].name = (*it);
		this->original_symbols[i].value = "";
#endif
	}
}

void Preprocessor::addSymbol(const PreprocSym& symbol)
{
	this->symbols.push_back(symbol);
#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	this->original_symbols.push_back(symbol);
#endif
}

const Preprocessor::SymbolList& Preprocessor::getSymbols() const
{
	return this->symbols;
}

uint Preprocessor::getNbSymbols() const
{
	return this->symbols.size();
}

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
const Preprocessor::SymbolList& Preprocessor::getOriginalSymbols() const
{
	return this->original_symbols;
}

uint Preprocessor::getNbOriginalSymbols() const
{
	return this->original_symbols.size();
}
#endif

bool Preprocessor::hasSymbol(const std::string& symbol_name) const
{
	SymbolList::const_iterator it_end = symbols.end();
	for(SymbolList::const_iterator it = symbols.begin() ;
		it != it_end ;
		it++)
	{
		const PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return true;
	}
	return false;
}

PreprocSym* Preprocessor::getSymbolByName(const std::string& symbol_name)
{
	SymbolList::iterator it_end = symbols.end();
	for(SymbolList::iterator it = symbols.begin() ;
		it != it_end ;
		it++)
	{
		PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return &sym;
	}
	return NULL;
}

const PreprocSym* Preprocessor::getSymbolByName(const std::string& symbol_name) const
{
	SymbolList::const_iterator it_end = symbols.end();
	for(SymbolList::const_iterator it = symbols.begin() ;
		it != it_end ;
		it++)
	{
		const PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return &sym;
	}
	return NULL;
}

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
bool Preprocessor::hasOriginalSymbol(const std::string& symbol_name) const
{
	SymbolList::const_iterator it_end = original_symbols.end();
	for(SymbolList::const_iterator it = original_symbols.begin() ;
		it != it_end ;
		it++)
	{
		const PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return true;
	}
	return false;
}

PreprocSym* Preprocessor::getOriginalSymbolByName(const std::string& symbol_name)
{
	SymbolList::iterator it_end = original_symbols.end();
	for(SymbolList::iterator it = original_symbols.begin() ;
		it != it_end ;
		it++)
	{
		PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return &sym;
	}
	return NULL;
}

const PreprocSym* Preprocessor::getOriginalSymbolByName(const std::string& symbol_name) const
{
	SymbolList::const_iterator it_end = original_symbols.end();
	for(SymbolList::const_iterator it = original_symbols.begin() ;
		it != it_end ;
		it++)
	{
		const PreprocSym& sym = (*it);
		if(sym.name == symbol_name)
			return &sym;
	}
	return NULL;
}
#endif

void Preprocessor::clearSymbols()
{
	symbols.clear();
#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
	original_symbols.clear();
#endif
}

// Get a string of the form 'SYMBOL_1="1", SYMBOL_2="0"' (useful for debugging)
std::string Preprocessor::getSymbolsString() const
{
	SymbolList::const_iterator it_end = symbols.end();

	stringstream ss;
	for(SymbolList::const_iterator it = symbols.begin() ;
		it != it_end ;
		)
	{
		const PreprocSym& sym = (*it);
		ss << sym.name << "=\"" << sym.value << "\"";

		it++;

		if(it != it_end)
			ss << ", ";
	}
	ss << flush;
	return ss.str();
}

#ifdef PREPROC_KEEP_ORIGINAL_SYMBOLS
std::string Preprocessor::getOriginalSymbolsString() const
{
	SymbolList::const_iterator it_end = original_symbols.end();

	stringstream ss;
	for(SymbolList::const_iterator it = original_symbols.begin() ;
		it != it_end ;
		)
	{
		const PreprocSym& sym = (*it);
		ss << sym.name << "=\"" << sym.value << "\"";

		it++;

		if(it != it_end)
			ss << ", ";
	}
	ss << flush;
	return ss.str();
}
#endif

// ---------------------------------------------------------------------
// Execute the preprocessor:
const char* Preprocessor::preprocess(const char* original_source, const char* filename)
{
	string result;

	// Retrieve the directory of the treated file (for the #include directive):
	assert(filename != NULL);
	string prev_directory = this->directory;	// we need to backup the value of this->directory
												// as this value changes while processing an included file.
	this->directory = retrieveDirectory(filename);

	const char* no_comments_source = removeComments(original_source, filename);

	if(no_comments_source == NULL)	// In case of error
		return NULL;

#ifdef DEBUG_PREPROCESSOR
	logDebug8("without comments [\"", filename, "\"]:");
	cout << no_comments_source << flush;
	logDebug8("--------");
#endif

	const char* line_ptr = no_comments_source;
	uint num_line = 1;
	uint nb_generated_lines = 0;
	list<bool> commented_out;

	// Preprocess the file line by line:
	while(*line_ptr != '\0')
	{
		// Preprocess line:
		const char* new_line_ptr = NULL;

		result += preprocessLine(line_ptr, filename, &num_line, &commented_out, &nb_generated_lines, &new_line_ptr);

		// If there is an error:
		if(nb_generated_lines == 0)
		{
			delete [] no_comments_source;
			return NULL;
		}

		// Go to either next line, or another line according to what the call to preprocessLine() has indicated:
		if(new_line_ptr != NULL)
			line_ptr = new_line_ptr;
		else
		{
			// Go to next line:
			const char* pc = NULL;
			for(pc = line_ptr ; *pc != '\n' ; pc++);
			line_ptr = &pc[1];
		}
	}

	delete [] no_comments_source;

	// Check if we are not in a #for block in the end:
	if(this->for_directive_line != NULL)
	{
		logError(filename, ": #for without #endfor");
		return NULL;
	}

	// Return a copy of the result (we can not return result.c_str(), which will be freed)
	uint len = result.length();
	char* result_copy = new char[len+1];
	memcpy(result_copy, result.c_str(), (len+1)*sizeof(char));

#ifdef DEBUG_PREPROCESSOR
	logDebug9("preproc result [\"", filename, "\"]:");
	cout << result_copy << flush;
	logDebug9("--------");
#endif

	// Restore the initial directory:
	this->directory = prev_directory;

	return result_copy;
}

const char* Preprocessor::preprocessFromFile(const char* filename)
{
	const char* original_source = loadText(filename);

	if(!original_source)
		return NULL;

	const char* result = preprocess(original_source, filename);
	delete [] original_source;

	return result;
}

// ---------------------------------------------------------------------
// Load a text file:
const char* Preprocessor::loadText(const char* filename) const
{
	ifstream f(filename);
	if(!f)
	{
		logError("impossible to read the file \"", filename, "\"");
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

// ---------------------------------------------------------------------
string Preprocessor::retrieveDirectory(const char* filename) const
{
	string directory = "";
	string uri_section = "";
	char c;

	for(const char* pc = filename ;
		*pc != '\0' ;
		pc++)
	{
		c = *pc;

		uri_section += c;

		if(c == '\\' || c == '/')
		{
			directory += uri_section;
			uri_section = "";
		}
	}

	if(directory == "")
		directory = "./";

	return directory;
}

// ---------------------------------------------------------------------
// Remove comments and set all newlines to '\n', and add a final '\n' if there is none
const char* Preprocessor::removeComments(const char* original_source, const char* filename)
{
	string result;

	uint num_line = 1;

	for(uint i=0 ; original_source[i] != '\0' ; i++)
	{
		char c = original_source[i];
		char next = original_source[i+1];

		// Skip '\r'
		if(c == '\r')
			continue;

		// "//" -> skip the rest of the line
		if(c == '/' && next == '/')
		{
			// Go to the next '\n' and next line
			while(original_source[i] != '\n' && original_source[i] != '\0')
				i++;
			result += '\n';
			num_line++;

			if(original_source[i] == '\0')
				break;
			else
				continue;
		}

		// "/*" -> skip until we reach the corresponding "*/"
		else if(c == '/' && next == '*')
		{
			uint num_line_comment_start = num_line;

			bool found = false;
			for(i=i+2 ; original_source[i] != '\0' ; i++)
			{
				char current_char = original_source[i];
				char next_char = original_source[i+1];

				if(current_char == '\n')
				{
					num_line++;
					result += '\n';
				}

				else if(current_char == '*' && next_char == '/')
				{
					found = true;
					i++;	// point to the '/', so that next iteration points to the next character
					break;
				}
			}

			if(!found)
			{
				logError(filename, ":", num_line_comment_start, ": \"/*\" without corresponding \"*/\"");
				return NULL;
			}
		}

		// New line:
		else if(c == '\n')
		{
			num_line++;
			result += '\n';
		}

		// Normal character: add it to the result
		else
			result += c;
	}

	// Add a final '\n' if there is none:
	if(result.empty() || result[result.length()-1] != '\n')
		result += '\n';

	// Return a copy of the result (we can not return result.c_str(), which will be freed)
	uint len = result.length();
	char* result_copy = new char[len+1];
	memcpy(result_copy, result.c_str(), (len+1)*sizeof(char));
	return result_copy;
}

// ---------------------------------------------------------------------
// Preprocess a line.
// NB: the line's end is marked with '\n'!
string Preprocessor::preprocessLine(const char* line,
									const char* filename,
									uint* num_line,
									list<bool>* commented_out,
									uint *nb_generated_lines,
									const char** new_line_ptr)
{
	// Preprocess line:
	string result;

	uint next_num_line = (*num_line) + 1;
	const char* first_non_space = skipStartingSpaces(line);
	*nb_generated_lines = 1;

	*new_line_ptr = NULL;	// default value

	// See if we are in a "commented out" state.
	// #include, #define, #undef, #for and #endfor are skipped
	// if we are in a "commented out" state.
	bool is_commented_out = hasAtLeastOneTrue(*commented_out);

	// Check if the line is a preprocessor directive:
	if(*first_non_space == '#')
	{
		bool ok = true;
		result = "\n";	// replace the directive by an empty line

		if(startsWith(first_non_space, "#include") && ! is_commented_out)
			ok = includeFile(first_non_space, filename, *num_line, &result, nb_generated_lines);
		else if(startsWith(first_non_space, "#define") && ! is_commented_out)
			ok = defineSymbol(first_non_space);
		else if(startsWith(first_non_space, "#undef") && ! is_commented_out)
			ok = undefSymbol(first_non_space);
		else if(startsWith(first_non_space, "#ifdef"))
		{
			bool is_defined = true;
			ok = definedExpression(first_non_space, &is_defined);
			commented_out->push_back(!is_defined);
		}
		else if(startsWith(first_non_space, "#ifndef"))
		{
			bool is_defined = true;
			ok = definedExpression(first_non_space, &is_defined);
			commented_out->push_back(is_defined);
		}
		else if(startsWith(first_non_space, "#if"))
		{
			bool is_true = true;
			ok = condExpression(first_non_space, &is_true);
			commented_out->push_back(!is_true);
		}
		else if(startsWith(first_non_space, "#else"))
		{
			// Make sure there is nothing but whitespaces after "#else"
			if(*skipStartingSpaces(&first_non_space[strlen("#else")]) != '\n')
				ok = false;
			commented_out->back() = !commented_out->back();
		}
		else if(startsWith(first_non_space, "#elif"))
		{
			bool is_true = true;
			ok = condExpression(first_non_space, &is_true);
			commented_out->back() = !is_true;
		}
		else if(startsWith(first_non_space, "#endif"))
		{
			// Make sure there is nothing but whitespaces after "#endif"
			if(*skipStartingSpaces(&first_non_space[strlen("#endif")]) != '\n')
				ok = false;
			commented_out->pop_back();
		}
		else if(startsWith(first_non_space, "#for") && ! is_commented_out)
			ok = forExpression(first_non_space, filename, *num_line);
		else if(startsWith(first_non_space, "#endfor") && ! is_commented_out)
			ok = endFor(first_non_space, new_line_ptr, &result, &next_num_line);
		else
		{
			// In case this is a preprocessor directive we do not handle, if we are not in
			// a "commented out" state, keep the line as it was:
			result = "";
			if( ! is_commented_out)
				for(const char* pc = first_non_space ; *pc != '\n' ; pc++)
					result += *pc;
			result += '\n';
		}

		// Error handling:
		if(!ok)
		{
			logError(filename, ":", *num_line, ": error in the preprocessor directive");
			*nb_generated_lines = 0;
			*num_line = next_num_line;
			return string("");
		}
	}
	// If the line is not a preprocessor directive:
	else
	{
		// If we are not in a "commented out" state, find all the occurences of known symbols in it,
		// and expand them.
		// Otherwise, just add a blank line.
		if( ! is_commented_out)
			result = replaceSymbols(line, false) + "\n";
		else	// commented out
			result = "\n";
	}

	*num_line = next_num_line;

	return result;
}

// Replace the symbols in an expression.
// If in_directive == true, this means replaceSymbols() is called for an expression
// inside a directive, in which case we do not replace symbols preceded with a "defined"
// NB: the end of the expression is marked with '\n' or '\0'!
string Preprocessor::replaceSymbols(const char* expr, bool in_directive)
{
	string identifier = "";
	string result = "";
	uint i=0;
	bool can_replace=true;	// this is for the special case of "defined": we don't want
							// to replace the identifiers following it if in_directive == true

	bool building_identifier = false;

	// For each character in the line:
	for(i=0 ; expr[i] != '\n' && expr[i] != '\0' ; i++)
	{
		char c = expr[i];

		// If we are NOT building an identifier:
		if(!building_identifier)
		{
			if(isAlpha(c))
			{
				// Start building an identifier:
				building_identifier = true;
				identifier = c;
			}
			// @: special character used for the #for counter -> replace with the counter's value
			else if(c == '@')
			{
				stringstream ss;
				ss << this->for_directive_counter;
				result += ss.str();
				can_replace = true;	// we can replace the next identifier
			}
			// Not related to an identifier nor to a #for counter: just copy
			else
			{
				result += c;
				if(!isWhitespace(c))
					can_replace = true;	// we can replace the next identifier
			}
		}

		// If we ARE building an identifier:
		else
		{
			if(isAlpha(c) || isDigit(c))
			{
				// Continue building the identifier:
				identifier += c;
			}
			else
			{
				// Stop building the identifier:
				building_identifier = false;

				// If the symbol is known, replace it by its value,
				// otherwise just copy the identifier
				PreprocSym* sym = NULL;
				if(can_replace && (sym = getSymbolByName(identifier)) != NULL)
					result += sym->value;
				else
					result += identifier;

				// If the identifier is "defined" and in_directive == true, mark that
				// we won't replace the following symbol.
				// In the other case, we can replace the next identifier.
				if(in_directive && identifier == "defined")
					can_replace = false;
				else
					can_replace = true;

				identifier = "";

				// Treat the character:
				// @: special character used for the #for counter -> replace with the counter's value
				if(c == '@')
				{
					stringstream ss;
					ss << this->for_directive_counter;
					result += ss.str();
					can_replace = true;
				}
				// Not related to an identifier nor to a #for counter: just copy
				else
				{
					result += c;
					if(!isWhitespace(c))
						can_replace = true;
				}
			}
		}
	}

	// If an identifier which was being built, we either copy it or replace it by its value if it is known.
	if(building_identifier)
	{
		PreprocSym* sym = NULL;
		if(can_replace && (sym = getSymbolByName(identifier)) != NULL)
			result += sym->value;
		else
			result += identifier;
	}

	return result;
}

// Input example: "#include "my_file.glsl""
bool Preprocessor::includeFile(	const char* directive,
								const char* filename,
								uint num_line,
								string* result,
								uint* nb_generated_lines)
{
	string header_filename = directory;
	uint i=0;
	stringstream ss;

	*nb_generated_lines = 1;

	// Read the name of the file to include:
	// - go to the first '"'
	for(i=strlen("#include") ; directive[i] != '\"' ; i++)
	{
		char c = directive[i];
		if(!isWhitespace(c) && c != '\"')
			return false;
	}

	// - read the file's name
	i++;	// skip '\"'
	for(; directive[i] != '\"' ; i++)
		header_filename += directive[i];

	// - make sure there is nothing else than whitespaces remaining
	i++;	// skip '\"'
	if(*skipStartingSpaces(&directive[i]) != '\n')
		return false;

	// Indicate that we changed the current file:
	uint prev_source_string_number = source_string_number;
	if(use_line_directive)
	{
		source_string_number++;
		ss << "#line 1 " << source_string_number << endl;
		*result += ss.str();
		ss.str("");
	}

	// Preprocess the file:
	const char* header_content = preprocessFromFile(header_filename.c_str());

	if(header_content == NULL)
		return false;

	// Add the new content to the current preprocessed file:
	*result += header_content;

	// Indicate we are back to the original file:
	if(use_line_directive)
	{
		ss << "\n#line " << num_line+1 << " " << prev_source_string_number << endl;
		*result += ss.str();
		ss.str("");
	}

	// Cleanup
	delete [] header_content;

	return true;	// no error
}

// Input examples: "#define MY_SYMBOL value"
//                 "#define MY_SYMBOL"
// (no starting space)
bool Preprocessor::defineSymbol(const char* directive)
{
	PreprocSym symbol;
	uint keyword_len = strlen("#define");

	// Make sure there is at least one whitespace after "#define"
	if(!isWhitespace(directive[keyword_len]))
		return false;

	// Get the symbol's name
	const char* symbol_name = skipStartingSpaces(&directive[keyword_len]);
	uint index = 0;
	for(index=0 ; !isWhitespace(symbol_name[index]) && symbol_name[index] != '\n' ; index++)
		symbol.name += symbol_name[index];

	// Get the symbol's value, replacing necessary symbols in it
	symbol.value = replaceSymbols(skipStartingSpaces(&symbol_name[index]), true);

	// Check that the symbol is not already known:
	if(hasSymbol(symbol.name))
	{
		logError("symbol \"", symbol.name, "\" already known");
		return false;
	}

	// Add the symbol to the list
	symbols.push_back(symbol);

	return true;	// no error
}

// Input examples: "#undef MY_SYMBOL value"
//                 "#undef  MY_SYMBOL"
// (no starting space)
bool Preprocessor::undefSymbol(const char* directive)
{
	string str_symbol_name = "";
	uint keyword_len = strlen("#undef");

	// Make sure there is at least one whitespace after "#undef"
	if(!isWhitespace(directive[keyword_len]))
		return false;

	// Get the symbol's name
	const char* symbol_name = skipStartingSpaces(&directive[keyword_len]);
	uint index = 0;
	for(index=0 ; !isWhitespace(symbol_name[index]) && symbol_name[index] != '\n' ; index++)
		str_symbol_name += symbol_name[index];

	// Remove the symbol from the list
	SymbolList::iterator it_end = symbols.end();
	for(SymbolList::iterator it = symbols.begin() ;
		it != it_end ;
		it++)
	{
		PreprocSym& sym = (*it);
		if(sym.name == str_symbol_name)
		{
			symbols.erase(it);
			return true;
		}
	}

	logError("symbol \"", symbol_name, "\" unknown");
	return false;	// error
}

// ---------------------------------------------------------------------
// Input examples:
// "#if SYMBOL1 == SYMBOL2"
// "#elif SYMBOL1 != SYMBOL2"
bool Preprocessor::condExpression(const char* directive, bool* is_true)
{
	string str_expression = "";
	uint keyword_len;

	if(startsWith(directive, "#if"))
		keyword_len = strlen("#if");
	else
		keyword_len = strlen("#else");

	// Make sure there is at least one whitespace after "#if" or "#elif"
	if(!isWhitespace(directive[keyword_len]))
		return false;

	// Get the expression to evaluate
	for(const char* pc = skipStartingSpaces(&directive[keyword_len]) ; *pc != '\n' ; pc++)
		str_expression += *pc;

	// Replace all symbols that could be replaced inside:
	str_expression = replaceSymbols(str_expression.c_str(), true);

	// Evaluate the expression:
	bool ok = true;
	int value = evaluator->evaluate(str_expression, &ok);
	if(!ok)
		return false;

	*is_true = (value != 0);

	return true;	// no error
}

// ---------------------------------------------------------------------
// Input examples:
// "#ifdef SYMBOL1"
// "#ifndef SYMBOL1"
bool Preprocessor::definedExpression(const char* directive, bool* is_defined)	// for #ifdef and #ifndef
{
	string str_symbol_name = "";
	uint keyword_len;

	if(startsWith(directive, "#ifdef"))
		keyword_len = strlen("#ifdef");
	else
		keyword_len = strlen("#ifndef");

	// Make sure there is at least one whitespace after "#ifdef" or "#ifndef"
	if(!isWhitespace(directive[keyword_len]))
		return false;

	// Get the name of the symbol. We remove the last '\n'.
	for(const char* pc = skipStartingSpaces(&directive[keyword_len]) ;
		*pc != '\n' ;
		pc++)
	{
		str_symbol_name += *pc;
	}
	str_symbol_name = removeEndingWhitespaces(str_symbol_name); // remove the last whitespaces

	// Check if the symbol's name is valid:
	if(!isValidSymbolName(str_symbol_name.c_str()))
	{
		logError("invalid symbol name: \"", str_symbol_name, "\"");
		return false;
	}

	// Check if the symbol exists:
	*is_defined = hasSymbol(str_symbol_name);

	return true;	// no error
}

// ---------------------------------------------------------------------
// Input example:
// "#for NB_LIGHTS"
bool Preprocessor::forExpression(const char* directive, const char* filename, uint num_line)
{
	string str_expression = "";

	// Check if we are not already inside another #for block:
	if(this->for_directive_line != NULL)
	{
		logError(filename, ":", num_line, ": nested #for is not allowed");
		return false;
	}

	// Make sure there is at least one whitespace after "#for":
	const uint keyword_len = strlen("#for");
	if(!isWhitespace(directive[keyword_len]))
		return false;

	// Get the expression to evaluate:
	for(const char* pc = skipStartingSpaces(&directive[keyword_len]) ; *pc != '\n' ; pc++)
		str_expression += *pc;

	// Replace all symbols that could be replaced inside:
	str_expression = replaceSymbols(str_expression.c_str(), true);

	// Evaluate the expression:
	bool ok = true;
	int value = evaluator->evaluate(str_expression, &ok);
	if(!ok)
		return false;

	// Do nothing for a negative number of iterations
	if(value <= 0)
		value = 0;

	// Indicate where is the line with the #for directive:
	this->for_directive_line = directive;

	// Start the counter:
	this->for_directive_counter = 0;

	// Indicate the number of iterations:
	this->for_directive_nb_iter = uint(value);

	// Indicate the line of this directive:
	this->for_directive_num_line = num_line;

	return true;	// no error
}

// ---------------------------------------------------------------------
// Input example:
// "#endfor"
bool Preprocessor::endFor(const char* directive, const char** new_line_ptr, string* result, uint* next_num_line)
{
	// Make sure there is nothing but whitespaces after "#endfor"
	if(*skipStartingSpaces(&directive[strlen("#endfor")]) != '\n')
		return false;

	// Increment the counter
	this->for_directive_counter++;

	// Check if we should continue the loop or exit from it:
	// - case where we continue the loop:
	if(this->for_directive_counter < this->for_directive_nb_iter)
	{
		// Go to the line just after the #for directive:
		const char* pc = this->for_directive_line;
		while(*pc != '\n')
			pc++;
		*new_line_ptr = &pc[1];

		// Reset the line counter:
		*next_num_line = this->for_directive_num_line + 1;
	}
	// - case where we end the loop:
	else
	{
		this->for_directive_counter = 0;
		this->for_directive_line = NULL;

		// #line directive:
		if(use_line_directive)
		{
			stringstream ss;
			ss << "#line " << (*next_num_line) << endl;
			*result = ss.str();
		}
	}

	return true;	// no error
}
