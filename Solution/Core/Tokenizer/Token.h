#pragma once

#include <string>

namespace LexicalAnalyzer{
	enum TOKEN_TYPES { INVALID, KEYWORD, RESERVED_IDENTIFIER, UNRESERVED_IDENTIFIER, INTEGER, FLOAT, STRING, OPERATOR, DELIMETER, END_OF_PROGRAM };

	class Token
	{
	public:
		struct string_coord {
			int col, row;
		};

		Token(string_coord, TOKEN_TYPES, std::string, void*);
		Token(string_coord);
		~Token();

		void setText(std::string);
		void setValue(void*);
		void setType(TOKEN_TYPES);

		void print();

		string_coord get_position_in_string();
		TOKEN_TYPES get_type();
		
	private:
		std::string text;
		void* value;
		TOKEN_TYPES type;

		string_coord position_in_string;
	};
}