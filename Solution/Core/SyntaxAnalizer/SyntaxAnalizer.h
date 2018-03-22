#pragma once

#include "../Tokenizer/Tokenizer.h"
#include "Node.h"

class SyntaxAnalizer {
public:
	SyntaxAnalizer(LexicalAnalyzer::Tokenizer*);

	Node* parseProgram();
	Node* parseSimpleExpression();
	Node* parseSimpleConstantExpression();

private:
	LexicalAnalyzer::Tokenizer* tokenizer;

	Node* parseProgramHeading();
	Node* parseTerm();
	Node* parseConstantTerm();
	Node* parseFactor();
	Node* parseConstantFactor();
	Node* parseBlock(bool = true);
	std::vector<Node*>* parseDeclarationPart(bool);
	Node* parseStatementPart();
	std::vector<Node*>* parseStatementSequence();
	Node* parseStatement();
	Node* parseSimpleStatement();
	Node* parseAssignmentStatement();
	Node* parseVariable();
	Node* parseEntireVariable();
	Node* parseEntireConstantVariable();
	Node* parseExpression();
	Node* parseConstantExpression();
	Node* parseStructuredStatement();
	Node* parseCompoundStatement();
	Node* parseRepititiveStatement();
	Node* parseWhileStatement();
	Node* parseRepeatStatement();
	Node* parseForStatement();
	Node* parseIfStatement();
	Node* parseComponentVariable();
	Node* parseIndexedVariable();
	std::vector<Node*>* parseExpressionList();
	Node* parseProcedureStatement();
	std::vector<Node*>* parseActualParameterList();
	std::vector<Node*>* parseConstantActualParameterList();
	Node* parseActualParameter();
	Node* parseConstantActualParameter();
	Node* parseFieldDesignator();
	Node* parseFunctionDesignator();
	Node* parseConstantFunctionDesignator();
	Node* parseConstantDefinitionPart();
	Node* parseVarDefinitionPart();
	Node* parseConstantDefinition();
	Node* parseVarDefinition();
	Node* parseType();
	Node* parseArrayDefinition();

	static const std::unordered_set<std::string> builtInTypes;

	void throwError(LexicalAnalyzer::Token::StringCoord, char*, std::string = "Syntax error");
};