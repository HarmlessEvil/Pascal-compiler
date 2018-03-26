#include "Token.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace LexicalAnalyzer;
using namespace std;

Token::Token(StringCoord pos, TokenType type, std::string text, void* value) : position_in_string(pos), type(type), text(text), value(value)
{
}

Token::Token(StringCoord pos) : position_in_string(pos), type(INVALID)
{
}

Token::~Token()
{
	if (value) {
		delete value;
	}
}

void Token::setText(std::string text)
{
	this->text = text;
}

void Token::setValue(void* value)
{
	switch (type) {
	case INTEGER_LITERAL:
		this->value = new long long int;
		memcpy(this->value, value, sizeof(long long int));
		break;
	case FLOAT_LITERAL:
		this->value = new long double;
		memcpy(this->value, value, sizeof(long double));
		break;
	default:
		this->value = value;
		break;
	}
}

void Token::setType(TokenType type)
{
	this->type = type;
}

void Token::print()
{
	const char separator  = ' ';
	const int  posWidth   = 10;
	const int  classWidth = 30;
	const int  valueWidth = 15;
	const int  textWidth  = 40;
	ostringstream pos_string, length_string;

	pos_string << '(' << position_in_string.row << ", " << position_in_string.col << ')';
	cout << left << setw(posWidth) << setfill(' ') << pos_string.str();
	cout << left << setw(classWidth) << setfill(separator);
	switch (type) {
	case KEYWORD:
	case BEGIN_KEYWORD:
	case END_KEYWORD:
	case WHILE_HEADING:
	case DO_KEYWORD:
	case REPEAT_HEADING:
	case UNTIL_KEYWORD:
	case FOR_HEADING:
	case FOR_UP_KEYWORD:
	case FOR_DOWN_KEYWORD:
	case IF_HEADING:
	case THEN_KEYWORD:
	case ELSE_KEYWORD:
	case DECLARATION_TYPE:
	case ARRAY_HEADING:
	case OF_KEYWORD:
	case NIL:
		cout << "Keyword";
		break;
	case RESERVED_IDENTIFIER:
		cout << "Reserved identifier";
		break;
	case UNRESERVED_IDENTIFIER:
		cout << "Unreserved identifier";
		break;
	case INTEGER_LITERAL:
		cout << "Integer";
		break;
	case FLOAT_LITERAL:
		cout << "Floating point number";
		break;
	case STRING_LITERAL:
		length_string << "String (" << text.length() - 2 << ')';
		cout << length_string.str();
		break;
	case OPERATOR:
	case ASSIGNMENT_OPERATOR:
	case RELATIONAL_OPERATOR:
	case ADDITION_OPERATOR:
	case MULTIPLICATION_OPERATOR:
		cout << "Operator";
		break;
	case SEMICOLON:
	case DOT:
	case LSBRACE:
	case RSBRACE:
	case COMMA:
	case LBRACE:
	case RBRACE:
	case DELIMETER:
		cout << "Delimeter";
		break;
	case RANGE:
		cout << "Range";
		break;
	case PROGRAM_HEADING:
		cout << "Program heading";
		break;
	case ENDOFFILE:
		cout << "EOF";
		break;
	case PROGRAM_BLOCK:
		cout << "Program block";
		break;
	case INVALID:
	default:
		cout << "Invalid type";
		break;
	}

	cout << left << setw(valueWidth) << setfill(separator);
	switch (type) {
	case INTEGER_LITERAL:
		cout << *(static_cast<long long int*>(value));
		break;
	case FLOAT_LITERAL:
		cout << *(static_cast<long double*>(value));
		break;
	default:
		cout << text;
		break;
	}

	cout << left << setw(textWidth) << setfill(separator) << text;
	cout << endl;
}

Token::StringCoord Token::getPositionInString()
{
	return position_in_string;
}

TokenType Token::getType()
{
	return type;
}

TokenType LexicalAnalyzer::Token::getSubType()
{
	switch (type) {
	case ADDITION_OPERATOR:
		if (text == "+") {
			return PLUS_OPERATOR;
		}
		else if (text == "-") {
			return MINUS_OPERATOR;
		}
		else if (text == "or") {
			return OR_OPERATOR;
		}
		break;

	case MULTIPLICATION_OPERATOR:
		if (text == "*") {
			return MULTIPLICATION_OPERATOR;
		}
		else if (text == "/") {
			return FDIV_OPERATOR;
		}
		else if (text == "div") {
			return IDIV_OPERATOR;
		}
		else if (text == "mod") {
			return MOD_OPERATOR;
		}
		else if (text == "and") {
			return AND_OPERATOR;
		}
		break;

	case RELATIONAL_OPERATOR:
		if (text == "=") {
			return EQUALS_RELATION_OPERATOR;
		}
		else if (text == "<>") {
			return NOT_EQUALS_RELATIONAL_OPERATOR;
		}
		else if (text == "<") {
			return LESS_RELATIONAL_OPERATOR;
		}
		else if (text == "<=") {
			return LESS_OR_EQUALS_RELATIONAL_OPERATOR;
		}
		else if (text == ">") {
			return GREATER_RELATIONAL_OPERATOR;
		}
		else if (text == ">=") {
			return GREATER_OR_EQUALS_RELATIONAL_OPERATOR;
		}
		else if (text == "in") {
			return IN_RELATIONAL_OPERATOR;
		}

	case DECLARATION_TYPE:
		if (text == "const") {
			return CONST_DEFINITION_KEYWORD;
		}
		else if (text == "var") {
			return VAR_DEFINITION_KEYWORD;
		}

	default:
		return TokenType();
	}

	return TokenType();
}

std::string LexicalAnalyzer::Token::getText()
{
	return this->text;
}

void* LexicalAnalyzer::Token::getValue()
{
	return this->value;
}

bool LexicalAnalyzer::Token::operator==(const char* c)
{
	return this->text.compare(c) == 0;
}

bool LexicalAnalyzer::Token::operator!=(const char* c)
{
	return this->text.compare(c) != 0;
}
