#pragma once

#include <fstream>
#include <map>

#include "Token.h"

namespace LexicalAnalyzer {
	class Tokenizer
	{
	public:
		Tokenizer(std::ifstream*);
		~Tokenizer();

		Token* next();
		Token* current();

	private:
		void print_error(int);

		std::ifstream* file;
		Token::string_coord curr_pos;
		Token* last_token;

		static const std::map<std::string, TOKEN_TYPES> keywords;
	};
}