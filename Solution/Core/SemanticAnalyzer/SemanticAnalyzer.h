#pragma once

#include <map>
#include <string>
#include <vector>

#include "../Tokenizer/Token.h"

class Symbol {
public:
	Symbol(std::string);

protected:
	std::string name;
};

class SymType : public Symbol {
public:
	SymType(std::string);
};
class SymTypeScalar : public SymType {
public:
	SymTypeScalar(std::string);
};

class SymTypeFloat : public SymTypeScalar {
public:
	SymTypeFloat(std::string);

	bool isCompatibleTo(LexicalAnalyzer::TokenType);
};

class SymTypeInteger : public SymTypeScalar {
public:
	SymTypeInteger(std::string);

	bool isCompatibleTo(LexicalAnalyzer::TokenType);
};

class SymVar : public Symbol {
public:
	SymVar(std::string, Symbol*);

	void isCompatibleTo(LexicalAnalyzer::Token*);

private:
	Symbol* type;
};

class SemanticAnalyzer {
public:
	static SymVar* addVariable(std::string, LexicalAnalyzer::Token*);

	static Symbol* getSymbol(std::string);

	static SymType* parseType(std::string);

	static bool symbolExistsInGlobalScope(std::string);

	static SymTypeInteger* getIntegerType();
	static SymTypeFloat* getFloatType();

	static void throwError(LexicalAnalyzer::Token::StringCoord, const char*);

private:
	static bool symbolExistsInLocalScope(std::string);

	static std::vector<std::map<std::string, Symbol*>> symbol_table_vector;
	static SymTypeInteger* integer_type;
	static SymTypeFloat* float_type;
};