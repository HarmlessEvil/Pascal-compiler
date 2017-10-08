#include "Token.h"

#include <iostream>
#include <iomanip>

using namespace LexicalAnalyzer;
using namespace std;

Token::Token()
{
}

Token::~Token()
{
}

void Token::print()
{
	const char separator  = '|';
	const int  posWidth   = 7;
	const int  classWidth = 15;
	const int  valueWidth = 15;
	const int  textWidth  = 40;

	cout << left << setw(posWidth) << setfill(' ') << '(' << position_in_string.col << ',' << position_in_string.row << ')';
	cout << left << setw(classWidth) << setfill(separator);
	switch (type) {
	case KEYWORD:
		cout << "Keyword";
		break;
	case RESERVED_IDENTIFIER:
		cout << "Reserved identifier";
		break;
	case UNRESERVED_IDENTIFIER:
		cout << "Unreserver identifier";
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