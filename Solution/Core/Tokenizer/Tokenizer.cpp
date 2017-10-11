#include <algorithm>
#include <cctype>
#include <iostream>

#include "Tokenizer.h"
#include "Errors.h"

using namespace std;
using namespace LexicalAnalyzer;

const map<string, TOKEN_TYPES> Tokenizer::keywords = {
	{ "begin", KEYWORD },
	{ "end",   KEYWORD },
	{ "end.",  END_OF_PROGRAM },
	{ "and", OPERATOR },
	{ "array", KEYWORD },
	{ "asm", KEYWORD },
	{ "break", KEYWORD },
	{ "case", KEYWORD },
	{ "const", KEYWORD },
	{ "constructor", KEYWORD },
	{ "continue", KEYWORD },
	{ "destructor", KEYWORD },
	{ "div", OPERATOR },
	{ "do", KEYWORD },
	{ "downto", KEYWORD },
	{ "else", KEYWORD },
	{ "end", KEYWORD },
	{ "file", KEYWORD },
	{ "for", KEYWORD },
	{ "function", KEYWORD },
	{ "goto", KEYWORD },
	{ "if", KEYWORD },
	{ "implementation", KEYWORD },
	{ "in", OPERATOR },
	{ "inline", KEYWORD },
	{ "interface", KEYWORD },
	{ "label", KEYWORD },
	{ "mod", OPERATOR },
	{ "nil", KEYWORD },
	{ "not", OPERATOR },
	{ "object", KEYWORD },
	{ "of", KEYWORD },
	{ "on", KEYWORD },
	{ "operator", KEYWORD },
	{ "or", OPERATOR },
	{ "packed", KEYWORD },
	{ "procedure", KEYWORD }, 
	{ "program", KEYWORD },
	{ "record", KEYWORD },
	{ "repeat", KEYWORD },
	{ "set", KEYWORD },
	{ "shl", OPERATOR },
	{ "shr", OPERATOR },
	{ "string", KEYWORD },
	{ "then", KEYWORD },
	{ "to", KEYWORD },
	{ "unit", KEYWORD },
	{ "until", KEYWORD },
	{ "uses", KEYWORD },
	{ "var", KEYWORD },
	{ "while", KEYWORD },
	{ "with", KEYWORD },
	{ "xor", OPERATOR },
	{ "abs", RESERVED_IDENTIFIER },
	{ "arctan", RESERVED_IDENTIFIER },
	{ "boolean", RESERVED_IDENTIFIER },
	{ "char", RESERVED_IDENTIFIER },
	{ "cos", RESERVED_IDENTIFIER },
	{ "dispose", RESERVED_IDENTIFIER },
	{ "eof", RESERVED_IDENTIFIER },
	{ "eoln", RESERVED_IDENTIFIER },
	{ "exp", RESERVED_IDENTIFIER },
	{ "false", RESERVED_IDENTIFIER },
	{ "abs", RESERVED_IDENTIFIER },
	{ "input", RESERVED_IDENTIFIER },
	{ "integer", RESERVED_IDENTIFIER },
	{ "ln", RESERVED_IDENTIFIER },
	{ "maxint", RESERVED_IDENTIFIER },
	{ "new", RESERVED_IDENTIFIER },
	{ "odd", RESERVED_IDENTIFIER },
	{ "ord", RESERVED_IDENTIFIER },
	{ "output", RESERVED_IDENTIFIER },
	{ "pack", RESERVED_IDENTIFIER },
	{ "page", RESERVED_IDENTIFIER },
	{ "pred", RESERVED_IDENTIFIER },
	{ "read", RESERVED_IDENTIFIER },
	{ "readln", RESERVED_IDENTIFIER },
	{ "ln", RESERVED_IDENTIFIER },
	{ "real", RESERVED_IDENTIFIER },
	{ "reset", RESERVED_IDENTIFIER },
	{ "rewrite", RESERVED_IDENTIFIER },
	{ "round", RESERVED_IDENTIFIER },
	{ "sin", RESERVED_IDENTIFIER },
	{ "sqr", RESERVED_IDENTIFIER },
	{ "sqrt", RESERVED_IDENTIFIER },
	{ "succ", RESERVED_IDENTIFIER },
	{ "text", RESERVED_IDENTIFIER },
	{ "true", RESERVED_IDENTIFIER },
	{ "trunc", RESERVED_IDENTIFIER },
	{ "write", RESERVED_IDENTIFIER },
	{ "writeln", RESERVED_IDENTIFIER },
};

const unordered_set<string> Tokenizer::operators = {
	"+", "-", "*", "/", "%", "=", "<>", ">", "<", ">=", "<=", "&", "|", "~", "!", "<<", ">>", "><", "+=", "*=", "-=", "/=", ":="
};

const unordered_set<char> Tokenizer::operators_parts = {
	'<', '>', '+', '*', '-', '/', ':', '='
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

void finish_integer(Token*& token, string& s) {
	token->setText(s);

	long long int n = strtoll(s.c_str(), NULL, 0);
	token->setType(INTEGER);
	token->setValue(&n);
}

void finish_hex(Token*& token, string& s) {
	token->setText(s);

	s[0] = 'x';
	s = '0' + s;
	long long int n = strtoll(s.c_str(), NULL, 0);
	token->setType(INTEGER);
	token->setValue(&n);
}

void finish_identifier(Token*& token, string& s, const std::map<std::string, TOKEN_TYPES>& keywords) {
	token->setText(s);

	transform(s.begin(), s.end(), s.begin(), ::tolower);

	auto kw = keywords.find(s);
	if (kw != keywords.end()) {
		token->setType(kw->second);
	}
	else {
		token->setType(UNRESERVED_IDENTIFIER);
	}

	token->setValue((void*)s.c_str());
}

void finish_float(Token*& token, string& s) {
	token->setText(s);

	long double n = stold(s.c_str(), NULL);
	token->setType(FLOAT);
	token->setValue(&n);
}

void finish_delimeter(Token*& token, string& s) {
	token->setText(s);
	token->setType(DELIMETER);
	token->setValue((void*)s.c_str());
}

void finish_operator(Token*& token, string& s) {
	token->setText(s);
	token->setType(OPERATOR);
	token->setValue((void*)s.c_str());
}

void finish_string(Token*& token, string& s) {
	token->setText(s);
	token->setType(STRING);
	token->setValue((void*)s.c_str());
}

void finish_range(Token*& token, string& s) {
	token->setText(s);
	token->setType(RANGE);
	token->setValue((void*)s.c_str());
}

Token* Tokenizer::next()
{
	char c = 0;
	string s("");
	Token* token = NULL;

	if (file->eof()) {
		return NULL;
	}
	
	bool idle = true;
	bool reading_identifier = false;
	bool reading_integer = false;
	bool reading_float = false;
	bool reading_exponent_e = false;
	bool reading_exponent_sign = false;
	bool reading_hex_sign = false;
	bool reading_hex = false;

	bool reading_slash = false;
	bool skipping_one_line_comment = false;

	bool skipping_multiline_comment = false;

	bool reading_lbrace = false;
	bool skipping_old_style_comments = false;
	bool reading_asteriks = false;

	bool reading_operator = false;
	bool reading_string = false;

	while (file->get(c)) {
		if (('\n' != c) && ('\t' != c) && (' ' != c) && !skipping_one_line_comment && !skipping_multiline_comment && !skipping_old_style_comments) {
			if (idle) {
				token = new Token(curr_pos);
				idle = false;
			}
			
			if ('\'' == c) {
				s += c;

				if (reading_string) {
					finish_string(token, s);

					last_token = token;
					return last_token;
				}
				else {
					reading_string = true;
				}
			}
			else if (reading_string) {
				s += c;
			}
			else if ('*' != c && reading_lbrace) {
				reading_lbrace = false;

				file->putback(c);

				s = "(";
				finish_delimeter(token, s);
				last_token = token;
				return last_token;
			}
			else if (s == "." && '.' != c) {
				file->putback(c);

				finish_delimeter(token, s);
				last_token = token;
				return last_token;
			}
			else if (reading_operator && operators.find(s + c) == operators.end()) {
				file->putback(c);

				finish_operator(token, s);
				last_token = token;
				return last_token;
			}
			else if (isalpha(c) || '_' == c) {
				if (reading_integer || reading_float) {
					if ('e' == tolower(c)) {
						reading_float = true;
						reading_exponent_e = true;
						reading_integer = false;

						s += c;
					}
					else {
						last_token = token;
						print_error(ILLEGAL_IDENTIFIER);
						return NULL;
					}
				}
				else if (reading_hex_sign || reading_hex) {
					if ('a' <= tolower(c) && tolower(c) <= 'f') {
						reading_hex_sign = false;
						reading_hex = true;

						s += c;
					}
				}
				else {
					reading_identifier = true;

					s += c;
				}
			}
			else if (isdigit(c)) {
				if (reading_identifier) {
					s += c;
				}
				else {
					if (!reading_float) {
						if (reading_hex_sign) {
							reading_hex_sign = false;
							reading_hex = true;
						}
						else if (!reading_hex) {
							reading_integer = true;
						}
					}
					else {
						reading_exponent_e = false;
						reading_exponent_sign = false;
					}

					s += c;
				}
			}
			else if ('.' == c) {

				if (iequals("end", s)) {
					s += c;

					token->setText(s);
					token->setType(END_OF_PROGRAM);
					last_token = token;
					return last_token;
				}
				else if (reading_identifier) {
					file->putback(c);

					finish_identifier(token, s, keywords);
					last_token = token;
					return last_token;
				}
				else if (reading_integer) {
					reading_integer = false;
					reading_float = true;

					s += c;
				}
				else if (reading_float) {
					if (s.back() == '.') {
						file->putback('.');
						file->putback('.');
						s.pop_back();

						finish_integer(token, s);
						last_token = token;
						return last_token;
					}
					else {
						last_token = token;
						print_error(ILLEGAL_FLOAT);
						return NULL;
					}
				}
				else if (reading_hex_sign) {
					last_token = token;
					print_error(ILLEGAL_HEX);
					return NULL;
				}
				else {
					if ('.' == c ) {
						if (s == ".") {
							s += c;

							finish_range(token, s);
							last_token = token;
							return last_token;
						}
						else {
							s += c;
						}
					}
				}
			}
			else if ('+' == c) {
				if (reading_exponent_e) {
					reading_exponent_e = false;
					reading_exponent_sign = true;

					s += c;
				}
			}
			else if ('$' == c) {
				reading_hex_sign = true;

				s += c;
			}
			else if ('/' == c) {
				if (reading_slash) {
					reading_slash = false;
					skipping_one_line_comment = true;
					s = "";
				}
				else {
					reading_slash = true;
					
					s += c;
				}
			}
			else if ('{' == c) {
				skipping_multiline_comment = true;
			}
			else if ('(' == c) {
				if (reading_identifier) {
					file->putback(c);

					finish_identifier(token, s, keywords);
					last_token = token;
					return last_token;
				}
				else if (reading_exponent_sign || reading_exponent_e) {
					last_token = token;
					print_error(ILLEGAL_FLOAT);
					return NULL;
				}
				else if (reading_hex_sign) {
					last_token = token;
					print_error(ILLEGAL_HEX);
					return NULL;
				}
				else if (reading_hex) {
					file->putback(c);

					finish_hex(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_integer) {
					file->putback(c);

					finish_integer(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_float) {
					file->putback(c);

					finish_float(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_slash) {
					file->putback(c);

					finish_operator(token, s);
					last_token = token;
					return last_token;
				}

				reading_lbrace = true;
			}
			else if ('*' == c && reading_lbrace) {
				reading_lbrace = false;
				skipping_old_style_comments = true;
			}
			else if (operators.find(string(c, 1)) != operators.end() || operators_parts.find(c) != operators_parts.end() || operators.find(s) != operators.end()) {
				if (reading_identifier) {
					file->putback(c);

					finish_identifier(token, s, keywords);
					last_token = token;
					return last_token;
				}
				else if (reading_exponent_sign) {
					last_token = token;
					print_error(ILLEGAL_FLOAT);
					return NULL;
				}
				else if (reading_hex_sign) {
					last_token = token;
					print_error(ILLEGAL_HEX);
					return NULL;
				}
				else if (reading_hex) {
					file->putback(c);

					finish_hex(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_integer) {
					file->putback(c);

					finish_integer(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_float && !reading_exponent_e) {
					file->putback(c);

					finish_float(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_slash && '=' != c) {
					file->putback(c);

					finish_operator(token, s);
					last_token = token;
					return last_token;
				}

				if (reading_float && reading_exponent_e) {
					if ('+' == c || '-' == c) {
						s += c;
					}
					else {
						last_token = token;
						print_error(ILLEGAL_FLOAT);
						return NULL;
					}
				}
				else {
					s += c;
				}

				if (!reading_operator) {
					if (operators_parts.find(c) == operators_parts.end()) {
						finish_operator(token, s);
						last_token = token;
						return last_token;
					}
					else if (reading_exponent_e && ('+' == c || '-' == c)) {
						reading_exponent_e = false;
						reading_exponent_sign = true;
					}
					else {
						reading_operator = true;
					}
				}
				else {
					finish_operator(token, s);
					last_token = token;

					curr_pos.col++;

					return last_token;
				}
			}
			else if (',' == c || ':' == c || ';' == c || ')' == c || '[' == c || ']' == c) {
				if (reading_identifier) {
					file->putback(c);

					finish_identifier(token, s, keywords);
					last_token = token;
					return last_token;
				}
				else if (reading_exponent_sign || reading_exponent_e) {
					last_token = token;
					print_error(ILLEGAL_FLOAT);
					return NULL;
				}
				else if (reading_hex_sign) {
					last_token = token;
					print_error(ILLEGAL_HEX);
					return NULL;
				}
				else if (reading_hex) {
					file->putback(c);

					finish_hex(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_integer) {
					file->putback(c);

					finish_integer(token, s);
					last_token = token;
					return last_token;
				}
				else if (reading_float) {
					file->putback(c);

					finish_float(token, s);
					last_token = token;
					return last_token;
				}

				s += c;

				curr_pos.col++;
				finish_delimeter(token, s);
				last_token = token;
				return last_token;
			}
		}
		else {
			if ('\n' == c) {
				curr_pos.col = 0;
				curr_pos.row++;

				if (reading_string) {
					print_error(MISS_QUOTE);
					return NULL;
				}
			}

			if (reading_string) {
				s += c;
			}
			else {
				if (s == "." && '.' != c) {
					finish_delimeter(token, s);
					last_token = token;

					curr_pos.col++;

					return last_token;
				}
				if (')' != c && reading_asteriks) {
					reading_asteriks = false;
				}
				else if (reading_operator) {
					if (s == ":") {
						finish_delimeter(token, s);
					}
					else {
						finish_operator(token, s);
					}
					curr_pos.col++;

					last_token = token;
					return last_token;
				}

				if (!skipping_one_line_comment && !skipping_multiline_comment && !skipping_old_style_comments) {
					if (!idle) {
						if (reading_identifier) {
							finish_identifier(token, s, keywords);
						}
						else if (reading_integer) {
							finish_integer(token, s);
						}
						else if (reading_exponent_sign || reading_exponent_e) {
							last_token = token;
							print_error(ILLEGAL_FLOAT);
							return NULL;
						}
						else if (reading_hex_sign) {
							last_token = token;
							print_error(ILLEGAL_HEX);
							return NULL;
						}
						else if (reading_hex) {
							finish_hex(token, s);
						}
						else if (reading_float) {
							finish_float(token, s);
						}

						curr_pos.col++;

						last_token = token;
						return last_token;
					}
				}

				if ('}' == c && skipping_multiline_comment) {
					skipping_multiline_comment = false;
					idle = true;
				}
				else if ('*' == c && skipping_old_style_comments) {
					reading_asteriks = true;
				}
				else if (')' == c && reading_asteriks) {
					skipping_old_style_comments = false;
					reading_asteriks = false;

					idle = true;
				}

				if (curr_pos.col == 0 && skipping_one_line_comment) {
					skipping_one_line_comment = false;
					idle = true;
				}
			}
			}

		curr_pos.col++;
	}

	if (!idle && !skipping_one_line_comment && !skipping_multiline_comment && !skipping_old_style_comments) {
		if (reading_identifier) {
			finish_identifier(token, s, keywords);
		}
		else if (reading_integer) {
			finish_integer(token, s);
		}
		else if (reading_exponent_sign || reading_exponent_e) {
			last_token = token;
			print_error(ILLEGAL_FLOAT);
			return NULL;
		}
		else if (reading_hex_sign) {
			last_token = token;
			print_error(ILLEGAL_HEX);
			return NULL;
		}
		else if (reading_hex) {
			finish_hex(token, s);
		}
		else if (reading_float) {
			finish_float(token, s);
		}

		curr_pos.col++;
		last_token = token;
		return last_token;
	}
	else {
		return NULL;
	}
}

Token* Tokenizer::current()
{
	return last_token;
}

void Tokenizer::print_error(int err_num)
{
	Token* token = current();
	cerr << "Lexical error at (" << token->get_position_in_string().row << ", " << token->get_position_in_string().col << "): " << error[err_num] << endl;
}
