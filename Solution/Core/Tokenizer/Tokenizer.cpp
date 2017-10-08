#include <algorithm>
#include <iostream>

#include "Tokenizer.h"
#include "Errors.h"

using namespace std;
using namespace LexicalAnalyzer;

const std::map<string, TOKEN_TYPES> Tokenizer::keywords = {
	{ "begin", KEYWORD },
	{ "end",   KEYWORD },
	{ "end.",  END_OF_PROGRAM }
};

Tokenizer::Tokenizer(ifstream* in) : curr_pos({1, 1}), last_token(NULL)
{
	file = in;
}

Tokenizer::~Tokenizer()
{
}

bool iequals(const string& a, const string& b)
{
	unsigned int sz = a.size();
	if (b.size() != sz)
		return false;
	for (unsigned int i = 0; i < sz; ++i)
		if (tolower(a[i]) != tolower(b[i]))
			return false;
	return true;
}

Token* Tokenizer::next()
{
	char c;
	string s("");
	Token* token = NULL;

	if (file->eof()) {
		return NULL;
	}
	
	bool idle = true;
	bool reading_identifier = false;

	while (file->get(c)) {
		if (('\n' != c) && (' ' != c)) {
			if (idle) {
				token = new Token(curr_pos);
				idle = false;
			}
			
			if (isalpha(c)) {
				reading_identifier = true;

				s += c;
			}
			else if (reading_identifier && '.' == c && iequals("end", s)) {
				s += c;

				token->setText(s);
				token->setValue((void*)s.c_str());
				token->setType(END_OF_PROGRAM);

				last_token = token;
				return last_token;
			}
		}
		else {
			if ('\n' == c) {
				curr_pos.col = 0;
				curr_pos.row++;
			}

			if (reading_identifier) {
				token->setText(s);
				token->setValue((void*)s.c_str());

				transform(s.begin(), s.end(), s.begin(), ::tolower);

				auto kw = keywords.find(s);
				if (kw != keywords.end()) {
					token->setType(kw->second);
				}
				else {
					token->setType(UNRESERVED_IDENTIFIER);
				}

				curr_pos.col++;

				last_token = token;
				return last_token;
			}
		}

		curr_pos.col++;
	}

	if (reading_identifier) {
		token->setText(s);
		token->setValue((void*)s.c_str());

		transform(s.begin(), s.end(), s.begin(), ::tolower);

		auto kw = keywords.find(s);
		if (kw != keywords.end()) {
			token->setType(kw->second);
		}
		else {
			token->setType(UNRESERVED_IDENTIFIER);
		}

		curr_pos.col++;

		last_token = token;
		return last_token;
	}
	else {
		last_token = new Token(curr_pos, INVALID, string(1, c), (void*)c);
		return last_token;
	}
}

Token* Tokenizer::current()
{
	return last_token;
}

void Tokenizer::print_error(int err_num)
{
	Token* token = current();
	cerr << "Lexical error at " << token->get_position_in_string().row << ' ' << token->get_position_in_string().col << ':' << error[err_num] << endl;
}
