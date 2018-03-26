#pragma once

#include <string>
#include <vector>

#include "../CodeGenerator/Assembler.h"
#include "../SemanticAnalyzer/SemanticAnalyzer.h"

enum NodeType { INTEGER_NODE, REAL_NODE };

class Node {
public:
	Node();
	Node(LexicalAnalyzer::Token*);
	Node(LexicalAnalyzer::Token*, std::vector<Node*>*);
	Node(LexicalAnalyzer::Token*, Node*, Node*);
	Node(LexicalAnalyzer::Token*, Node*);
	~Node();

	virtual void generate();

	std::vector<Node*>* children;
	virtual Node* attach(Node*, bool = false);

	virtual void call();

	virtual void setType(SymType*);
	virtual SymType* getType();

	LexicalAnalyzer::Token* get_token();

	bool isConstant();
	void print();

protected:
	LexicalAnalyzer::Token* token;

	bool constant = false;
	bool isCallable = false;
	void print_tree(Node*, int);

	SymType* symType;
};

class ProgramHeadingNode : public Node {
public:
	ProgramHeadingNode(LexicalAnalyzer::Token*);
private:
	std::string programName;
};

class ProgramNode : public Node {
public:
	ProgramNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	Node *heading, *body;
};

class SimpleExpressionNode : public Node {
public:
	SimpleExpressionNode(LexicalAnalyzer::Token*, Node*, Node*);

	SymType* getType();

	void generate();
private:
	Node *lhs, *rhs;
	LexicalAnalyzer::TokenType additionalOperator;
};

class SimpleConstantExpressionNode : public Node {
public:
	SimpleConstantExpressionNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	Node *lhs, *rhs;
	LexicalAnalyzer::TokenType additionalOperator;
};

class TermNode : public Node {
public:
	TermNode(LexicalAnalyzer::Token*, Node*, Node*);

	SymType* getType();

	void generate();
private:
	Node *lhs, *rhs;
	LexicalAnalyzer::TokenType multiplicationOperator;
};

class ConstantTermNode : public Node {
public:
	ConstantTermNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	Node *lhs, *rhs;
	LexicalAnalyzer::TokenType multiplicationOperator;
};

class FactorNode : public Node {
public:
	FactorNode(LexicalAnalyzer::Token*, SymType*);

	SymType* getType();
	void generate();
private:
	LexicalAnalyzer::TokenType type;
};

class ConstantFactorNode : public Node {
public:
	ConstantFactorNode(LexicalAnalyzer::Token*);

	SymType* getType();
private:
	LexicalAnalyzer::TokenType type;
};

class StatementPartNode : public Node {
public:
	StatementPartNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
	void generate();
};

class AssignmentNode : public Node {
public:
	AssignmentNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	Node *lhs, *rhs;
};

class EntireVariableNode : public Node {
public:
	EntireVariableNode(LexicalAnalyzer::Token*);

	SymType* getType();
	void call();
	void generate();

	virtual Node* attach(Node*, bool = false);

private:
	std::vector<Node*>* parameters;
	std::vector<SymType*>* parameterTypes;
};

class EntireConstantVariableNode : public Node {
public:
	EntireConstantVariableNode(LexicalAnalyzer::Token*);
};

class ExpressionNode : public Node {
public:
	ExpressionNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	LexicalAnalyzer::TokenType relationalOperator;
	Node *lexpr, *rexpr;
};

class ConstantExpressionNode : public Node {
public:
	ConstantExpressionNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	LexicalAnalyzer::TokenType relationalOperator;
	Node *lexpr, *rexpr;
};

class CompoundStatementNode : public Node {
public:
	CompoundStatementNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
};

class WhileNode : public Node {
public:
	WhileNode(LexicalAnalyzer::Token*, Node*, Node*);
private:
	Node *condition, *statement;
};

class RepeatNode : public Node {
public:
	RepeatNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
private:
	Node* condition;
	std::vector<Node*>* statements;
};

class ForNode : public Node {
public:
	ForNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
private:
	Node *loopVariable, *initialValue, *finalValue, *body;
};

class IfNode : public Node {
public:
	IfNode(LexicalAnalyzer::Token*, Node*, Node*);
	IfNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
private:
	Node *condition, *expressionIfCondition, *elseExpression;
};

class IndicesOfVariableNode : public Node {
public:
	IndicesOfVariableNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
private:
	std::vector<Node*>* indices;
};

class ProcedureStatementNode : public Node {
public:
	ProcedureStatementNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
};

class FieldDesignatorNode : public Node {
public:
	FieldDesignatorNode(LexicalAnalyzer::Token*, Node*);
private:
	Node* field;
};

class FunctionDesignatorParametersNode : public Node {
public:
	FunctionDesignatorParametersNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
};

class ConstantFunctionDesignatorParametersNode : public Node {
public:
	ConstantFunctionDesignatorParametersNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
};

class ProgramBlockNode : public Node {
public:
	ProgramBlockNode(LexicalAnalyzer::Token*, std::vector<Node*>*, Node*);
private:
	std::vector<Node*>* declarations;
	Node* statements;
};

class ConstantDefinitionPartNode : public Node {
public:
	ConstantDefinitionPartNode(LexicalAnalyzer::Token*, std::vector<Node*>*);
};

class ConstantNode : public Node {
public:
	ConstantNode(LexicalAnalyzer::Token*, Node*, ConstantNode*);

private:
	std::string name;
	NodeType type;
	void* value;
};
