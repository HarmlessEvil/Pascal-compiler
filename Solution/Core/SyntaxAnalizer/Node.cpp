#include "Node.h"

#include <iostream>

using namespace std;
using namespace LexicalAnalyzer;

Node::Node()
{

}

Node::Node(Token* token) {
	this->token = token;

	this->children = new vector<Node*>();
}

Node::Node(Token* token, std::vector<Node*>* children)
{
	this->token = token;
	this->children = children;
}

Node::Node(Token* token, Node* left, Node* right)
{
	this->token = token;

	this->children = new vector<Node*>({left, right});
}

Node::Node(Token* token, Node* child)
{
	this->token = token;
	this->children = new vector<Node*>({child});
}

Node::~Node()
{
	this->children->clear();
	delete this->children;
}

Node* Node::attach(Node* child)
{
	this->children->push_back(child);
	return this;
}

LexicalAnalyzer::Token* Node::get_token()
{
	return token;
}

bool Node::isConstant()
{
	return this->constant;
}

void Node::print()
{
	print_tree(this, 0);
}

void Node::print_tree(Node* tree, int level)
{
	if (tree) {
		for (int i = 0; i < level - 1; i++) cout << "|  ";
		if (level) {
			cout << (char)192 << "--";
		}

		if (tree->token->getType() == BEGIN_KEYWORD) {
			cout << "block" << endl;
		}
		else {
			cout << tree->token->getText() << endl;
		}

		int len = tree->children->size();
		for (int i = 0; i < len; i++) {
			print_tree(tree->children->at(i), level + 1);
		}
	}
}

ProgramHeadingNode::ProgramHeadingNode(Token* token) : Node(token), programName(token->getText()) {}

ProgramNode::ProgramNode(Token* token, Node* heading, Node* body) : Node(token, heading, body), heading(heading), body(body) {}

SimpleExpressionNode::SimpleExpressionNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), additionalOperator(token->getSubType()), lhs(lhs), rhs(rhs) {}

SimpleConstantExpressionNode::SimpleConstantExpressionNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), additionalOperator(token->getSubType()), lhs(lhs), rhs(rhs) 
{
	this->constant = true;
}

TermNode::TermNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), lhs(lhs), rhs(rhs), multiplicationOperator(token->getSubType()) {}

ConstantTermNode::ConstantTermNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), lhs(lhs), rhs(rhs), multiplicationOperator(token->getSubType()) 
{
	this->constant = true;
}

FactorNode::FactorNode(Token* token) : Node(token), type(token->getType()) {}

ConstantFactorNode::ConstantFactorNode(Token* token) : Node(token), type(token->getType()) 
{
	this->constant = true;
}

StatementPartNode::StatementPartNode(Token* token, vector<Node*>* statements) : Node(token, statements) {}

AssignmentNode::AssignmentNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), lhs(lhs), rhs(rhs) {}

EntireVariableNode::EntireVariableNode(Token* token) : Node(token) {}

EntireConstantVariableNode::EntireConstantVariableNode(Token* token) : Node(token) 
{
	this->constant = true;
}

ExpressionNode::ExpressionNode(Token* token, Node* lexpr, Node* rexpr) : Node(token, lexpr, rexpr), relationalOperator(token->getSubType()), lexpr(lexpr), rexpr(rexpr) {}

CompoundStatementNode::CompoundStatementNode(LexicalAnalyzer::Token* token, std::vector<Node*>* statements) : Node(token, statements) {}

WhileNode::WhileNode(LexicalAnalyzer::Token* token, Node* condition, Node* statement): Node(token, condition, statement), condition(condition), statement(statement->children->at(0)) {}

RepeatNode::RepeatNode(LexicalAnalyzer::Token* token, std::vector<Node*>* statements) : Node(token, statements) 
{
	this->condition = statements->back()->children->at(0);
	this->statements = new vector<Node*>(*statements);
	this->statements->pop_back();
}

ForNode::ForNode(LexicalAnalyzer::Token* token, std::vector<Node*>* children) : Node(token, children), finalValue(children->at(1)->children->at(0)), body(children->at(2)->children->at(0))
{
	Node* assignment = children->at(0);

	this->loopVariable = assignment->children->at(0);
	this->initialValue = assignment->children->at(1);
}

IfNode::IfNode(LexicalAnalyzer::Token* token, Node* condition, Node* expressionIfCondition) : 
	Node(token, condition, expressionIfCondition), condition(condition), expressionIfCondition(expressionIfCondition->children->at(0)) {}

IfNode::IfNode(LexicalAnalyzer::Token* token, std::vector<Node*>* children) : 
	Node(token, children), condition(children->at(0)), expressionIfCondition(children->at(1)->children->at(0)), elseExpression(children->at(2)->children->at(0)) {}

IndicesOfVariableNode::IndicesOfVariableNode(LexicalAnalyzer::Token* brace, std::vector<Node*>* indices) : Node(brace, indices), indices(indices) {}

ProcedureStatementNode::ProcedureStatementNode(LexicalAnalyzer::Token* name, std::vector<Node*>* parameters) : Node(name, parameters) {}

FieldDesignatorNode::FieldDesignatorNode(LexicalAnalyzer::Token* dot, Node* field) : Node(dot, field), field(field) {}

FunctionDesignatorParametersNode::FunctionDesignatorParametersNode(LexicalAnalyzer::Token* lbrace, std::vector<Node*>* parameters) : Node(lbrace, parameters) {}

ConstantFunctionDesignatorParametersNode::ConstantFunctionDesignatorParametersNode(LexicalAnalyzer::Token* lbrace, std::vector<Node*>* parameters) : Node(lbrace, parameters) 
{
	this->constant = true;
}

ProgramBlockNode::ProgramBlockNode(LexicalAnalyzer::Token* dummy, vector<Node*>* declarations, Node* statements) : declarations(declarations), statements(statements) 
{
	this->token = dummy;

	declarations->push_back(statements);
	this->children = declarations;
}

ConstantDefinitionPartNode::ConstantDefinitionPartNode(LexicalAnalyzer::Token* token, std::vector<Node*>* constants) : Node(token, constants) {}

ConstantNode::ConstantNode(LexicalAnalyzer::Token* t_equals, Node* identifier, ConstantNode* value) : Node(t_equals, identifier, value)
{
	Token* token = identifier->get_token();

	this->name = token->getText();

	//TODO: Свёртка констант
}

ConstantExpressionNode::ConstantExpressionNode(Token* token, Node* lexpr, Node* rexpr) : Node(token, lexpr, rexpr), relationalOperator(token->getSubType()), lexpr(lexpr), rexpr(rexpr) 
{
	this->constant = true;
}
