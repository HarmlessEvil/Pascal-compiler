#pragma once

#include <fstream>
#include <map>
#include <unordered_set>

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
		static const std::unordered_set<std::string> operators;
		static const std::unordered_set<char> operators_parts;
	};
}