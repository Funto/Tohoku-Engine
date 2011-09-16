// ExpressionEvaluator.h

#ifndef EXPRESSION_EVALUATOR_H
#define EXPRESSION_EVALUATOR_H

#include <string>
#include <assert.h>
#include <list>
#include "Preprocessor.h"

/*	Supported operators:
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

class ExpressionEvaluator
{
private:
	const Preprocessor* preproc;	// Pointer to the calling preprocessor

	// Token management:
	enum TokenType
	{
		TOKEN_OPERATOR,
		TOKEN_IDENTIFIER,
		TOKEN_INT,
	};

	struct Token
	{
		Token(const char* str_val, TokenType type)
		: type(type), str_val(str_val), int_val(0)
		{
			assert(type != TOKEN_INT);
		}
		Token(int int_val)
		: type(TOKEN_INT), str_val(""), int_val(int_val)
		{
		}

		TokenType type;

		std::string str_val;
		int int_val;
	};

	typedef std::list<Token> TokenList;

public:
	ExpressionEvaluator(const Preprocessor* preproc);

	int evaluate(const std::string& expr, bool* ok) const;

private:
	bool makeTokenList(TokenList* tokens, const std::string& expr) const;

	int evaluateTokenList(TokenList& tokens, bool* ok) const;

	// Evaluate a binary operation: a+b, a-b, a*b...etc.
	// simple_operators example: "+-"
	// double_operators example: "<<>>"
	int evaluateBinaryOperation(	const TokenList& tokens,
									bool* ok,
									bool* found,
									const char* simple_operators,
									const char* double_operators) const;

	// For debugging:
	void printTokenList(const TokenList& tokens) const;
};

#endif // EXPRESSION_EVALUATOR_H
