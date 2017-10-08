#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include "Core\Tokenizer\Tokenizer.h"

using namespace std;
using namespace LexicalAnalyzer;

void help();

int main(int argc, char* argv[]) {
	bool lexer_only = false;
	ifstream* source_code = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "/l") == 0) {
			lexer_only = true;
		}
		else {
			source_code = new ifstream(argv[i]);
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
	while ((token = tokenizer->next()) && token->get_type() != END_OF_PROGRAM) {
		token->print();
	}
	if (token) {
		token->print();
	}

	if (lexer_only) {
		source_code->close();
		exit(EXIT_SUCCESS);
	}

	source_code->close();
}

void help() {
	cout << "Pascal compiler" << endl;
	cout << "Author: Alexander Chori<alexandrchori@gmail.com>, 2017" << endl;
	cout << "Usage: solution.exe [/l] file_name" << endl;
	cout << "  /l - lexical analysis only" << endl;
}