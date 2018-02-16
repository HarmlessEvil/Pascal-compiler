#pragma once

#include <map>
#include <string>

class Value {

};

class SemanticAnalyzer {
public:
	static std::map<std::string, Value> symbol_table;
};