// PreprocHelpers.h

#ifndef PREPROCHELPERS_H
#define PREPROCHELPERS_H

#include <string>
#include <cstring>
#include <list>
#include "../Common.h"

static inline const char* skipStartingSpaces(const char* line);	// return a pointer. No allocation performed.
static inline bool startsWith(const char* str, const char* starter);
static inline bool isAlpha(char c);
static inline bool isDigit(char c);
static inline bool isNumber(const char* str);
static inline bool isValidSymbolName(const char* str);
static inline bool isWhitespace(char c);
static inline std::string removeEndingWhitespaces(const std::string& str);
static inline bool hasAtLeastOneTrue(const std::list<bool>& l);
static inline bool isCharInString(char c, const char* s);
static inline bool isOperatorKnown(const char* operator_str, const char* simple_operators, const char* double_operators);

// ---------------------------------------------------------------------
static inline const char* skipStartingSpaces(const char* line)
{
	const char* ptr = line;
	while(isWhitespace(*ptr))
		ptr++;
	return ptr;
}

// ---------------------------------------------------------------------
static inline bool startsWith(const char* str, const char* starter)
{
	for(uint i=0 ; starter[i] != '\0' ; i++)
	{
		if(str[i] == '\0')	// len(str) < len(starter) ?
			return false;
		if(starter[i] != str[i])
			return false;
	}
	return true;
}

// ---------------------------------------------------------------------
static inline bool isAlpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// ---------------------------------------------------------------------
static inline bool isDigit(char c) {
	return (c >= '0' && c <= '9');
}

// ---------------------------------------------------------------------
static inline bool isNumber(const char* str)
{
	if(!isDigit(str[0]) && str[0] != '-')
		return false;

	for(uint i=1 ; str[i] != '\0' ; i++)
		if(!isDigit(str[i]))
			return false;
	return true;
}

// ---------------------------------------------------------------------
static inline bool isValidSymbolName(const char* str)
{
	if(!isAlpha(*str))
		return false;
	for(uint i=1 ; str[i] != '\0' ; i++)
		if(!isAlpha(str[i]) && !isDigit(str[i]))
			return false;
	return true;
}

// ---------------------------------------------------------------------
static inline bool isWhitespace(char c) {
	return (c == ' ') || (c == '\t');
}

// ---------------------------------------------------------------------
static inline std::string removeEndingWhitespaces(const std::string& str)
{
	if(str.length() == 0)
		return std::string("");

	int index = int(str.length()-1);
	while(index >= 0 && isWhitespace(str[index]))
		index--;

	return str.substr(0, index+1);
}

// ---------------------------------------------------------------------
static inline bool hasAtLeastOneTrue(const std::list<bool>& l)
{
	std::list<bool>::const_iterator it_end = l.end();
	for(std::list<bool>::const_iterator it = l.begin() ;
		it != it_end ;
		it++)
	{
		if(*it)
			return true;
	}
	return false;
}

// ---------------------------------------------------------------------
static inline bool isCharInString(char c, const char* s)
{
	for(const char* pc = s ; *pc != '\0' ; pc++)
		if(*pc == c)
			return true;
	return false;
}

// ---------------------------------------------------------------------
static inline bool isOperatorKnown(const char* operator_str, const char* simple_operators, const char* double_operators)
{
	uint len = std::strlen(operator_str);
	if(len == 1)
		return isCharInString(operator_str[0], simple_operators);
	else if(len == 2)
	{
		for(uint i=0 ; double_operators[i] != '\0' ; i += 2)
		{
			if(operator_str[0] == double_operators[i] &&
			   operator_str[1] == double_operators[i+1])
				return true;
		}
		return false;
	}
	else
	{
		return false;	// we do not know any operator with 3 characters (except "defined", but we don't
						// want it to be recognized here ^^)
	}
}

#endif // PREPROCHELPERS_H
