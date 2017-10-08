#pragma once

#include <fstream>

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
	};
}