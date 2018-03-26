#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Core\Tokenizer\Tokenizer.h"
#include "Core\SyntaxAnalizer\SyntaxAnalizer.h"

using namespace std;
using namespace LexicalAnalyzer;

void help();

int main(int argc, char* argv[]) {
	bool lexer_only = false;
	bool syntax_only = false;
	ifstream* source_code = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "/l") == 0) {
			lexer_only = true;
		}
		else if (strcmp(argv[i], "/s") == 0) {
			syntax_only = true;
		}
		else {
			source_code = new ifstream(argv[i]);
			AsmCode::set_output_path(argv[i]);
		}
	}
	if (source_code == NULL) {
		help();
		exit(EXIT_SUCCESS);
	}
	else if (source_code->fail()) {
		cout << "Error opening source code";
		exit(EXIT_FAILURE);
	}

	Tokenizer* tokenizer = new Tokenizer(source_code);
	Token* token = NULL;

	if (lexer_only) {
		while ((token = tokenizer->next()) && token->getType() != ENDOFFILE) {
			token->print();
		}
		if (token) {
			token->print();
		}

		source_code->close();
		exit(EXIT_SUCCESS);
	}

	SyntaxAnalizer* syntaxAnalizer = new SyntaxAnalizer(tokenizer);
	Node* AST = NULL;
	try {
		//AST = syntaxAnalizer->parseSimpleExpression();
		AST = syntaxAnalizer->parseProgram();
	}
	catch (exception &e) {
		cout << e.what();
	}
	AST->print();

	if (syntax_only) {
		source_code->close();
		exit(EXIT_SUCCESS);
	}

	source_code->close();
}

void help() {
	cout << "Pascal compiler" << endl;
	cout << "Author: Alexander Chori<alexandrchori@gmail.com>, 2017-2018" << endl;
	cout << "Usage: pascal.exe [/l] [/s] file_name" << endl;
	cout << "  /l - lexical analysis only" << endl;
	cout << "  /s - syntax analysis only" << endl;
}