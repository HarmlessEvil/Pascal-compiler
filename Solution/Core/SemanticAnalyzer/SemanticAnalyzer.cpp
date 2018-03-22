#include "SemanticAnalyzer.h"

#include <algorithm>
#include <string>
#include <sstream>

using namespace std;

using namespace LexicalAnalyzer;

SymTypeInteger* SemanticAnalyzer::integer_type = new SymTypeInteger("Integer");
SymTypeFloat* SemanticAnalyzer::float_type = new SymTypeFloat("Float");

vector<map<string, Symbol*>> SemanticAnalyzer::symbol_table_vector = {
	{
		{ "integer", integer_type },
		{ "float", float_type },
		{ "real", float_type }
	},
	{}
};

SymVar* SemanticAnalyzer::addVariable(std::string name, Token* token_type)
{
	if (symbolExistsInLocalScope(name)) {
		ostringstream s;

		s << "Variable " << name << " is already defined";
		throwError(token_type->getPositionInString(), s.str().c_str());
	}

	if (SymType* type = parseType(token_type->getText())) {
		SymVar* var = new SymVar(name, type);
		symbol_table_vector.back()[name] = var;

		return var;
	}

	throwError(token_type->getPositionInString(), "Failed to resolve type");
	return nullptr;
}

Symbol* SemanticAnalyzer::getSymbol(std::string name)
{
	transform(name.begin(), name.end(), name.begin(), ::tolower);
	for (auto it = symbol_table_vector.crbegin(); it != symbol_table_vector.crend(); it++) {
		auto s = it->find(name);

		if (s != it->end()) {
			return s->second;
		}
	}

	return nullptr;
}

SymType* SemanticAnalyzer::parseType(std::string name)
{
	return static_cast<SymType*>(getSymbol(name));
}

bool SemanticAnalyzer::symbolExistsInGlobalScope(string name)
{
	for (auto it = symbol_table_vector.crbegin(); it != symbol_table_vector.crend(); it++) {
		if (it->count(name) > 0) {
			return true;
		}
	}

	return false;
}

SymTypeInteger *SemanticAnalyzer::getIntegerType()
{
	return integer_type;
}

SymTypeFloat *SemanticAnalyzer::getFloatType()
{
	return float_type;
}

bool SemanticAnalyzer::symbolExistsInLocalScope(string name)
{
	return symbol_table_vector.back().count(name) > 0;
}

void SemanticAnalyzer::throwError(Token::StringCoord pos, const char* text)
{
	ostringstream s;
	s << "Semantic error" << " at (" << pos.row << ", " << pos.col << "): " << text;

	throw exception(s.str().c_str());
}

SymTypeFloat::SymTypeFloat(std::string name) : SymTypeScalar(name) {}

bool SymTypeFloat::isCompatibleTo(TokenType type)
{
	return type == FLOAT_LITERAL;
}

SymTypeInteger::SymTypeInteger(std::string name) : SymTypeScalar(name) {}

bool SymTypeInteger::isCompatibleTo(TokenType type)
{
	return type == INTEGER_LITERAL;
}

Symbol::Symbol(string name)
{
	this->name = name;
}

void SymVar::isCompatibleTo(Token* token)
{
	switch (token->getType()) {
	case INTEGER_LITERAL:
		if (this->type == SemanticAnalyzer::getIntegerType()) {
			return;
		}
		break;

	case FLOAT_LITERAL:
		if (this->type == SemanticAnalyzer::getFloatType()) {
			return;
		}
		break;
	}

	SemanticAnalyzer::throwError(token->getPositionInString(), "Types are incompatible");
}

SymTypeScalar::SymTypeScalar(string name) : SymType(name) {}

SymType::SymType(string name) : Symbol(name) {}

SymVar::SymVar(string name, Symbol* type) : Symbol(name) {
	this->type = type;
}
