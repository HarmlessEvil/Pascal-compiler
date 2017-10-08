#pragma once

#include <string>

namespace LexicalAnalyzer{
	enum TOKEN_TYPES { INVALID, KEYWORD, RESERVED_IDENTIFIER, UNRESERVED_IDENTIFIER, INTEGER, FLOAT, STRING, OPERATOR, DELIMETER };

	class Token
	{
	public:
		Token();
		~Token();

		std::string text;
		TOKEN_TYPES type;
		void* value;
		void print();

		struct string_coord {
			int col, row;
		};

		string_coord position_in_string;
	};
}