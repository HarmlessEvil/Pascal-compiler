#pragma once

namespace LexicalAnalyzer {
	#define UNDEFINED_ERROR 0
	#define ILLEGAL_IDENTIFIER 1
	#define ILLEGAL_FLOAT 2
	#define ILLEGAL_HEX 3
	#define MISS_QUOTE 4
	#define ILLEGAL_OCT 5
	#define ILLEGAL_BIN 6

	const char* error[] = {
		"Undefined error",
		"Illegal identifier name",
		"Illegal floating point constant",
		"Illegal hexademical constant",
		"Unmatched string quote",
		"Illegal octagonal constant",
		"Illegal binary constant"
	};
}