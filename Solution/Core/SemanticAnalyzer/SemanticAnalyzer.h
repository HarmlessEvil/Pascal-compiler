#pragma once

#include <map>
#include <string>
#include <vector>

#include "../Tokenizer/Token.h"

class Symbol {
public:
	Symbol(std::string);

	virtual size_t getSize();

protected:
	std::string name;
};

class SymType : public Symbol {
public:
	SymType(std::string);

	virtual bool isCompatibleTo(SymType*);
	virtual SymType* commonType(SymType*);
};
class SymTypeScalar : public SymType {
public:
	SymTypeScalar(std::string);
};

class SymFunc : public Symbol {
public:
	SymFunc(std::string);
	void act();
};

class SymTypeFloat : public SymTypeScalar {
public:
	SymTypeFloat(std::string);

	bool isCompatibleTo(SymType*);
	SymType* commonType(SymType*);
	size_t getSize();
};

class SymTypeInteger : public SymTypeScalar {
public:
	SymTypeInteger(std::string);

	bool isCompatibleTo(SymType*);
	SymType* commonType(SymType*);
	size_t getSize();
};

class SymVar : public Symbol {
public:
	SymVar(std::string, SymType*);

	SymType* getType();

private:
	SymType* type;
};

class SemanticAnalyzer {
public:
	static SymVar* addVariable(std::string, LexicalAnalyzer::Token*);
	static SymVar* getVariable(std::string);
	static SymFunc* getFunction(std::string);

	static Symbol* getSymbol(std::string);

	static SymType* parseType(std::string);

	static bool symbolExistsInGlobalScope(std::string);

	static size_t getSymTableSize();

	static SymTypeInteger* getIntegerType();
	static SymTypeFloat* getFloatType();

	//static void setTokenizer(LexicalAnalyzer::Tokenizer*);
	//static LexicalAnalyzer::Tokenizer* getTokenizer();

	static void throwError(LexicalAnalyzer::Token::StringCoord, const char*);
	static void throwError(const char*);

private:
	static bool symbolExistsInLocalScope(std::string);

	static std::vector<std::map<std::string, Symbol*>> symbol_table_vector;
	static SymTypeInteger* integer_type;
	static SymTypeFloat* float_type;

	//static LexicalAnalyzer::Tokenizer* tokenizer;
	//—сылка на неразрешЄнный внешний символ?
};