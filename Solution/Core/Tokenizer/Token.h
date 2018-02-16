#pragma once

#include <string>

namespace LexicalAnalyzer{
	enum TokenType { INVALID, KEYWORD, RESERVED_IDENTIFIER, UNRESERVED_IDENTIFIER, INTEGER, FLOAT, STRING, OPERATOR, DELIMETER, RANGE, PROGRAM_HEADING, SEMICOLON,
		ENDOFFILE, BEGIN_KEYWORD, END_KEYWORD, ASSIGNMENT_OPERATOR, DOT, WHILE_HEADING, RELATIONAL_OPERATOR, DO_KEYWORD, REPEAT_HEADING, UNTIL_KEYWORD, FOR_HEADING,
		FOR_UP_KEYWORD, FOR_DOWN_KEYWORD, IF_HEADING, THEN_KEYWORD, ELSE_KEYWORD, ADDITION_OPERATOR, MULTIPLICATION_OPERATOR, NIL, LSBRACE, RSBRACE, COMMA, LBRACE, RBRACE, PLUS_OPERATOR,
		MINUS_OPERATOR, OR_OPERATOR, MUL_OPERATOR, FDIV_OPERATOR, IDIV_OPERATOR, MOD_OPERATOR, AND_OPERATOR, EQUALS_RELATION_OPERATOR, NOT_EQUALS_RELATIONAL_OPERATOR, LESS_RELATIONAL_OPERATOR,
		LESS_OR_EQUALS_RELATIONAL_OPERATOR, GREATER_RELATIONAL_OPERATOR, GREATER_OR_EQUALS_RELATIONAL_OPERATOR, IN_RELATIONAL_OPERATOR, PROGRAM_BLOCK, CONST_DEFINITION_KEYWORD, 
		DECLARATION_TYPE};

	class Token
	{
	public:
		struct StringCoord {
			int col, row;
		};

		Token(StringCoord, TokenType, std::string, void*);
		Token(StringCoord);
		~Token();

		void setText(std::string);
		void setValue(void*);
		void setType(TokenType);

		void print();

		StringCoord getPositionInString();
		TokenType getType();
		TokenType getSubType();
		std::string getText();

		bool operator==(const char*);
		bool operator!=(const char*);
		
	private:
		std::string text;
		void* value;
		TokenType type;

		StringCoord position_in_string;
	};
}