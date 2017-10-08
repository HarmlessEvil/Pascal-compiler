#include <iostream>

#include "Tokenizer.h"
#include "Errors.h"

using namespace std;
using namespace LexicalAnalyzer;

Tokenizer::Tokenizer(ifstream* in)
{
}

Tokenizer::~Tokenizer()
{
}

Token* Tokenizer::next()
{
	return new Token();
}

Token* Tokenizer::current()
{
	return new Token();
}

void Tokenizer::print_error(int err_num)
{
	Token* token = current();
	cerr << "Lexical error at " << token->position_in_string.col << ' ' << token->position_in_string.row << ':' << error[err_num] << endl;
}
