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

SymVar* SemanticAnalyzer::getVariable(std::string name)
{
	SymVar* var = nullptr;

	try {
		var = static_cast<SymVar*>(getSymbol(name));

		if (var == nullptr) {
			throwError(/*tokenizer->current()->getPositionInString(), */("Variable " + name + " is undeclared").c_str());
		}
	}
	catch (exception e) { //Invalid cast exception???
		throwError(/*tokenizer->current()->getPositionInString(), */("Variable " + name + " is undeclared").c_str());
	}

	return var;
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

/*void SemanticAnalyzer::setTokenizer(LexicalAnalyzer::Tokenizer* tokenizer) 
{
	SemanticAnalyzer::tokenizer = tokenizer;
}

LexicalAnalyzer::Tokenizer * SemanticAnalyzer::getTokenizer()
{
	return SemanticAnalyzer::tokenizer;
}*/

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

void SemanticAnalyzer::throwError(const char* text)
{
	ostringstream s;
	s << "Semantic error : " << text;

	throw exception(s.str().c_str());
}

SymTypeFloat::SymTypeFloat(std::string name) : SymTypeScalar(name) {}

bool SymTypeFloat::isCompatibleTo(SymType* type)
{
	return type == SemanticAnalyzer::getFloatType() || SemanticAnalyzer::getIntegerType();
}

SymType* SymTypeFloat::commonType(SymType* type)
{
	if (this == type) {
		return type;
	}

	if (!this->isCompatibleTo(type)) {
		SemanticAnalyzer::throwError(/*SemanticAnalyzer::getTokenizer()->current()->getPositionInString(), */"Types are incompatible");
	}

	if (type == SemanticAnalyzer::getIntegerType()) {
		return this;
	}

	return nullptr;
}

SymTypeInteger::SymTypeInteger(std::string name) : SymTypeScalar(name) {}

bool SymTypeInteger::isCompatibleTo(SymType* type)
{
	return type == SemanticAnalyzer::getIntegerType();
}

SymType* SymTypeInteger::commonType(SymType* type)
{
	if (this == type) {
		return type;
	}

	if (!this->isCompatibleTo(type)) {
		SemanticAnalyzer::throwError(/*SemanticAnalyzer::getTokenizer()->current()->getPositionInString(), */"Types are incompatible");
	}

	if (type == SemanticAnalyzer::getFloatType()) {
		return type;
	}

	return nullptr;
}

Symbol::Symbol(string name)
{
	this->name = name;
}

SymType* SymVar::getType()
{
	return this->type;
}

SymTypeScalar::SymTypeScalar(string name) : SymType(name) {}

SymType::SymType(string name) : Symbol(name) {}

bool SymType::isCompatibleTo(SymType* type)
{
	return this->isCompatibleTo(type);
}

SymType* SymType::commonType(SymType* type)
{
	return this->commonType(type);
}

SymVar::SymVar(string name, SymType* type) : Symbol(name) {
	this->type = type;
}
