// ExpressionEvaluator.cpp

#include "ExpressionEvaluator.h"
#include "PreprocHelpers.h"
#include "../Common.h"
#include "../log/Log.h"
#include <cstdlib>
using namespace std;

//#define DEBUG_EXPR_EVALUATOR

ExpressionEvaluator::ExpressionEvaluator(const Preprocessor* preproc)
: preproc(preproc)
{
}

// ---------------------------------------------------------------------
int ExpressionEvaluator::evaluate(const string& expr, bool* ok) const
{
	// Make a list of tokens:
	TokenList tokens;
	*ok = makeTokenList(&tokens, expr);

	if( ! (*ok))
		return 0;

#ifdef DEBUG_EXPR_EVALUATOR
	printTokenList(tokens);
#endif

	return evaluateTokenList(tokens, ok);
}

// ---------------------------------------------------------------------
bool ExpressionEvaluator::makeTokenList(TokenList* tokens, const string& expr) const
{
	uint len = expr.length();

	for(uint i=0 ; true ; i++)
	{
		// Skip whitespaces:
		for(; isWhitespace(expr[i]) ; i++);

		// TERMINATION CRITERION IS HERE:
		// If after skipping whitespaces, we hit the end of the line, terminate.
		if(i == len)
			return true;

		char c = expr[i];
		char next = expr[i+1];

		// Case of operators (except for "defined"):
		if(	c == '(' || c == ')' || c == '+' || c == '-' || c == '~' ||
			c == '*' || c == '/' || c == '%' || c == '^')
		{
			char s[] = {c, '\0'};
			tokens->push_back(Token(s, TOKEN_OPERATOR));
		}
		else if(c == '<' || c == '>')	// <, >, <<, >>, <=, >=
		{
			// Error handling "<>" and "><":
			if((c == '<' && next == '>') ||
			   (c == '>' && next == '<'))
			{
				return false;
			}

			// <<, >>, <=, >=
			if(next == '=' || next == '<' || next == '>')
			{
				char s[] = {c, next, '\0'};
				tokens->push_back(Token(s, TOKEN_OPERATOR));
				i++;
			}
			else
			{
				char s[] = {c, '\0'};
				tokens->push_back(Token(s, TOKEN_OPERATOR));
			}
		}
		else if(c == '=')	// ==
		{
			if(next != '=')
				return false;
			tokens->push_back(Token("==", TOKEN_OPERATOR));
			i++;
		}
		else if(c == '!')	// !, !=
		{
			if(next == '=')
			{
				tokens->push_back(Token("!=", TOKEN_OPERATOR));
				i++;
			}
			else
				tokens->push_back(Token("!", TOKEN_OPERATOR));
		}
		else if(c == '&')	// &, &&
		{
			if(next == '&')
			{
				tokens->push_back(Token("&&", TOKEN_OPERATOR));
				i++;
			}
			else
				tokens->push_back(Token("&", TOKEN_OPERATOR));
		}
		else if(c == '|')	// |, ||
		{
			if(next == '|')
			{
				tokens->push_back(Token("||", TOKEN_OPERATOR));
				i++;
			}
			else
				tokens->push_back(Token("|", TOKEN_OPERATOR));
		}

		// Other cases than an operator:
		else
		{
			// Case of a number:
			if(isDigit(expr[i]))
			{
				// Read all digits in a string
				string s;
				for(; isDigit(expr[i]) ; i++)
					s += expr[i];
				i--;	// in the end, point to the last character which is a digit

				// Convert the string to an integer
				tokens->push_back(Token(atoi(s.c_str())));
			}
			else if(isAlpha(expr[i]))	// Case of an identifier:
			{
				// Read the identifier
				string s;
				for(; isAlpha(expr[i]) ; i++)
					s += expr[i];
				i--;	// in the end, point to the last character which is part of the identifier

				// Add the token to the list.
				// Special case is "defined", which is not an identifier but a symbol:
				if(s == "defined")
					tokens->push_back(Token(s.c_str(), TOKEN_OPERATOR));
				else
					tokens->push_back(Token(s.c_str(), TOKEN_IDENTIFIER));
			}
			else	// Unknown symbol
			{
				logError("character \'", expr[i], "\' found");
				return false;
			}
		}
	}

	return true;	// no error
}

// ---------------------------------------------------------------------
int ExpressionEvaluator::evaluateTokenList(TokenList& tokens, bool* ok) const
{
#define RAISE_ERROR()	\
	do {				\
		logError("when evaluating expression");	\
		*ok = false;	\
		return 0;		\
	} while(0)


	if(tokens.size() == 0)
		RAISE_ERROR();

	// Evaluate:
	// - case of a single token:
	if(tokens.size() == 1)
	{
		Token& token = tokens.front();

		// - identifier, which has not been replaced by the preprocessor beforehand => 0
		if(token.type == TOKEN_IDENTIFIER)
			return 0;
		// - number:
		else if(token.type == TOKEN_INT)
			return token.int_val;
		// - operator => error
		else
			RAISE_ERROR();
	}
	// - other cases:
	else
	{
		Token& first = tokens.front();
		Token& last = tokens.back();

		bool found = false;
		int temp_val = 0;

		// a||b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "", "||");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a&&b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "", "&&");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a|b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "|", "");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a^b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "^", "");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a&b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "&", "");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a==b
		// a!=b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "", "==!=");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a<b
		// a>b
		// a<=b
		// a>=b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "<>", "<=>=");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a<<b
		// a>>b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "", "<<>>");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a+b
		// a-b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "+-", "");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// a*b
		// a/b
		// a%b
		temp_val = evaluateBinaryOperation(tokens, ok, &found, "*/%", "");
		if( ! (*ok)) RAISE_ERROR();
		if(found) return temp_val;

		// Unary operators:
		// +a
		// -a
		// ~a
		// !a
		if(	first.str_val == "+" ||
			first.str_val == "-" ||
			first.str_val == "~" ||
			first.str_val == "!")
		{
			char c = first.str_val[0];

			tokens.pop_front();
			int value = evaluateTokenList(tokens, ok);

				 if(c == '+') return +value;
			else if(c == '-') return -value;
			else if(c == '~') return ~value;
			else if(c == '!') return !value;
			else              assert(false);
		}

		// defined a
		if(tokens.size() == 2 && first.str_val == "defined" && last.type == TOKEN_IDENTIFIER)
			return preproc->hasSymbol(last.str_val);

		// (a)
		if(first.str_val == "(" && last.str_val == ")")
		{
			tokens.pop_front(); tokens.pop_back();
			return evaluateTokenList(tokens, ok);
		}
	}

	// If we reached this: error:
	RAISE_ERROR();

#undef RAISE_ERROR
}

// ---------------------------------------------------------------------
// Evaluate a binary operation: a+b, a-b, a*b...etc.
// simple_operators example: "+-"
// double_operators example: "<<>>"
// Operators which are inside parenthesis are ignored.
int ExpressionEvaluator::evaluateBinaryOperation(	const TokenList& tokens,
													bool* ok,
													bool* found,
													const char* simple_operators,
													const char* double_operators) const
{
	*found = false;
	TokenList left_list;
	TokenList right_list;
	string operator_str = "";
	int parenthesis_level = 0;	// used for ignoring operators which are inside parenthesis (parenthesis_level != 0)

	TokenList::const_reverse_iterator rit = tokens.rbegin();
	TokenList::const_reverse_iterator rit_end = tokens.rend();

	// Make up the list of tokens which are in the left and right parts of the expression.
	// Associativity being from left to right for all binary operators,
	// we treat things so that:
	// - left part: all tokens until the last occurence of a token of interest
	// - right part: what remains.
	// Example:
	// 1+2+3
	// left part: 1+2
	// right part: 3

	// Reverse loop through the tokens, and stop when we find one of the treated operators while not
	// being inside parenthesis:
	for( ; !(*found) && rit != rit_end ; rit++)
	{
		const Token& token = *rit;

		// Update parenthesis_level and check that it is always positive
		if(token.str_val == ")")
			parenthesis_level++;
		else if(token.str_val == "(")
		{
			parenthesis_level--;
			if(parenthesis_level < 0)
			{
				logError("incorrect parenthesis");
				*ok = false;
				return 0;
			}
		}

		// If this token is one of the treated operators and we are not inside parenthesis,
		// stop the loop at the next iteration:
		if(	parenthesis_level == 0 &&
			token.type == TOKEN_OPERATOR &&
			isOperatorKnown(token.str_val.c_str(), simple_operators, double_operators))
		{
			operator_str = token.str_val;
			*found = true;
		}
		// Otherwise, add the token to the list of tokens we are constructing:
		else
			right_list.push_front(token);
	}

	// If we did not find any interesting operator, give up:
	if( ! (*found))
		return 0;

	// In case we found an interesting operator:
	else
	{
		// Read the left part of the expression:
		for(; rit != rit_end ; rit++)
			left_list.push_front(*rit);

		// In case the operator is "+" or "-",
		// we need to check that there is something on the left side, otherwise we messed
		// up with the unary "+" and "-":
		if(operator_str == "+" || operator_str == "-")
		{
			if(left_list.size() == 0)
			{
				*found = false;
				return 0;
			}
		}


		// Evaluate the left and the right part of the expression:
		int left_val = evaluateTokenList(left_list, ok);
		if( ! (*ok))
			return 0;
		int right_val = evaluateTokenList(right_list, ok);
		if( ! (*ok))
			return 0;

		// Finally combine the left and right part of the expression:
		if(operator_str == "+") return left_val + right_val;
		else if(operator_str == "-") return left_val - right_val;
		else if(operator_str == "*") return left_val * right_val;
		else if(operator_str == "/")
		{
			if(right_val == 0)
			{
				logError("division by zero!");
				*ok = false;
				return 0;
			}
			else
				return left_val / right_val;
		}
		else if(operator_str == "%")
		{
			if(right_val == 0)
			{
				logError("division by zero!");
				*ok = false;
				return 0;
			}
			else
				return left_val % right_val;
		}
		else if(operator_str == "<<") return left_val << right_val;
		else if(operator_str == ">>") return left_val >> right_val;
		else if(operator_str == "<")  return left_val < right_val;
		else if(operator_str == ">")  return left_val > right_val;
		else if(operator_str == "<=") return left_val <= right_val;
		else if(operator_str == ">=") return left_val >= right_val;
		else if(operator_str == "==") return left_val == right_val;
		else if(operator_str == "!=") return left_val != right_val;
		else if(operator_str == "&")  return left_val & right_val;
		else if(operator_str == "^")  return left_val ^ right_val;
		else if(operator_str == "|")  return left_val | right_val;
		else if(operator_str == "&&") return left_val && right_val;
		else if(operator_str == "||") return left_val || right_val;
		else
		{
			logError("preprocessor bug: operator \'", operator_str, "\' not treated");
			assert(false);
		}
	}

	// We should not reach this:
	assert(false);
	return 0;
}

// ---------------------------------------------------------------------
// For debugging:
void ExpressionEvaluator::printTokenList(const TokenList& tokens) const
{
	TokenList::const_iterator it_end = tokens.end() ;
	for(TokenList::const_iterator it = tokens.begin();
		it != it_end ;
		it++)
	{
		const Token& token = *it;
		logDebug3("type:", token.type, ", int_val=", token.int_val, ", str_val=\"", token.str_val, "\"");
	}
}
