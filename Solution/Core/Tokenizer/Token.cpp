#include "Token.h"

#include <iostream>
#include <sstream>
#include <iomanip>

using namespace LexicalAnalyzer;
using namespace std;

Token::Token(string_coord pos, TOKEN_TYPES type, std::string text, void* value) : position_in_string(pos), type(type), text(text), value(value)
{
}

Token::Token(string_coord pos) : position_in_string(pos), type(INVALID)
{
}

Token::~Token()
{
}

void Token::setText(std::string text)
{
	this->text = text;
}

void Token::setValue(void* value)
{
	this->value = value;
}

void Token::setType(TOKEN_TYPES type)
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
	ostringstream pos_string;

	pos_string << '(' << position_in_string.row << ", " << position_in_string.col << ')';
	cout << left << setw(posWidth) << setfill(' ') << pos_string.str();
	cout << left << setw(classWidth) << setfill(separator);
	switch (type) {
	case KEYWORD:
		cout << "Keyword";
		break;
	case RESERVED_IDENTIFIER:
		cout << "Reserved identifier";
		break;
	case UNRESERVED_IDENTIFIER:
		cout << "Unreserved identifier";
		break;
	case INTEGER:
		cout << "Integer";
		break;
	case FLOAT:
		cout << "Floating point number";
		break;
	case STRING:
		cout << "String (" << text.length() << ')';
		break;
	case OPERATOR:
		cout << "Operator";
		break;
	case DELIMETER:
		cout << "Delimeter";
		break;
	case END_OF_PROGRAM:
		cout << "End of program";
		break;
	case INVALID:
	default:
		cout << "Invalid type";
		break;
	}

	cout << left << setw(valueWidth) << setfill(separator);
	switch (type) {
	case FLOAT:
		cout << *(static_cast<long double*>(value));
		break;
	default:
		cout << text;
		break;
	}

	cout << left << setw(textWidth) << setfill(separator) << text;
	cout << endl;
}

Token::string_coord Token::get_position_in_string()
{
	return position_in_string;
}

TOKEN_TYPES Token::get_type()
{
	return type;
}
