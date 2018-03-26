#include "Node.h"

#include <iostream>
#include <sstream>

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

void Node::generate() {}

Node* Node::attach(Node* child, bool isCallable)
{
	this->children->push_back(child);
	this->isCallable = isCallable;
	return this;
}

void Node::call() {}

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

void Node::setType(SymType* type)
{
	this->symType = type;
}

SymType* Node::getType()
{
	return nullptr;
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

SymType* SimpleExpressionNode::getType()
{
	return lhs->getType()->commonType(rhs->getType());
}

void SimpleExpressionNode::generate()
{
	lhs->generate();
	rhs->generate();

	AsmCode::addCode(new AsmCommand("pop", EBX));
	AsmCode::addCode(new AsmCommand("pop", EAX));

	switch (this->additionalOperator) {
	case PLUS_OPERATOR:
		AsmCode::addCode(new AsmCommand("add", EAX, EBX));
		break;

	case MINUS_OPERATOR:
		AsmCode::addCode(new AsmCommand("sub", EAX, EBX));
		break;
	}

	AsmCode::addCode(new AsmCommand("push", EAX));
}

SimpleConstantExpressionNode::SimpleConstantExpressionNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), additionalOperator(token->getSubType()), lhs(lhs), rhs(rhs) 
{
	this->constant = true;
}

TermNode::TermNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), lhs(lhs), rhs(rhs), multiplicationOperator(token->getSubType()) {}

SymType* TermNode::getType()
{
	return lhs->getType()->commonType(rhs->getType());
}

void TermNode::generate()
{
	lhs->generate();
	rhs->generate();

	AsmCode::addCode(new AsmCommand("pop", EBX));
	AsmCode::addCode(new AsmCommand("pop", EAX));

	switch (this->multiplicationOperator) {
	case MULTIPLICATION_OPERATOR:
		AsmCode::addCode(new AsmCommand("imul", EBX));
		break;

	case MOD_OPERATOR:
	case IDIV_OPERATOR:
		AsmCode::addCode(new AsmCommand("cdq"));
		AsmCode::addCode(new AsmCommand("idiv", EBX));
		break;
	}

	if (this->multiplicationOperator == MOD_OPERATOR) {
		AsmCode::addCode(new AsmCommand("push", EDX));
	}
	else {
		AsmCode::addCode(new AsmCommand("push", EAX));
	}
}

ConstantTermNode::ConstantTermNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), lhs(lhs), rhs(rhs), multiplicationOperator(token->getSubType()) 
{
	this->constant = true;
}

FactorNode::FactorNode(Token* token, SymType* symType) : Node(token), type(token->getType()) {
	this->symType = symType;
}

SymType* FactorNode::getType()
{
	return symType;
}

void FactorNode::generate()
{
	if (type == LexicalAnalyzer::INTEGER_LITERAL) {
		int *val = static_cast<int*>(this->token->getValue());
		AsmCode::addCode(new AsmCommand("push", *val));
	}
}

ConstantFactorNode::ConstantFactorNode(Token* token) : Node(token), type(token->getType()) 
{
	this->constant = true;
}

SymType* ConstantFactorNode::getType()
{
	if (type == LexicalAnalyzer::INTEGER_LITERAL) {
		return SemanticAnalyzer::getIntegerType();
	}
	else {
		return SemanticAnalyzer::getFloatType();
	}

	//Несовместимые типы операндов: SymTypeInteger* и SymTypeFloat* --- ??
	//return type == LexicalAnalyzer::INTEGER_LITERAL ? SemanticAnalyzer::getIntegerType() : SemanticAnalyzer::getFloatType();
}

StatementPartNode::StatementPartNode(Token* token, vector<Node*>* statements) : Node(token, statements) {}

void StatementPartNode::generate()
{
	AsmCode::addCode(new AsmLabel("__@function0"));
	AsmCode::addCode(new AsmCommand("enter", SemanticAnalyzer::getSymTableSize(), 1));
	
	for (auto it = this->children->begin(); it != this->children->end(); it++) {
		Node* node = *it;
		if (node) {
			node->generate();
		}
	}

	AsmCode::addCode(new AsmCommand("leave"));
	AsmCode::addCode(new AsmCommand("ret", 0));
}

AssignmentNode::AssignmentNode(Token* token, Node* lhs, Node* rhs) : Node(token, lhs, rhs), lhs(lhs), rhs(rhs) {}

EntireVariableNode::EntireVariableNode(Token* token) : Node(token) {}

SymType* EntireVariableNode::getType()
{
	if (this->symType == nullptr) {
		this->symType = SemanticAnalyzer::getVariable(token->getText())->getType();
	}

	return this->symType;
}

void EntireVariableNode::call()
{
	if (!isCallable) {
		return;
	}

	SymFunc* function = SemanticAnalyzer::getFunction(this->token->getText());
}

void EntireVariableNode::generate()
{
	static int counter = 0;

	if (isCallable) {
		ostringstream format;
		size_t size = 0;

		for (auto &type : *(this->parameterTypes)) {
			if (type == SemanticAnalyzer::getIntegerType()) {
				format << "37,100,32"; // %d
				size += 4;
			}
			else if (type == SemanticAnalyzer::getFloatType()) {
				format << "37,102,32"; // %l
				size += 8;
			}
		}
		if (this->parameterTypes->size() > 0) {
			format << ",";
		}
		format << "10,0"; // \n & null-terminator

		ostringstream name;
		name << "__@stringformat" << ++counter;

		AsmCode::addConstant(new AsmConstant(name.str(), "db", format.str()));

		for (auto it = this->parameters->rbegin(); it != this->parameters->rend(); it++) {
			(*it)->generate();
		}

		AsmCode::addCode(new AsmCommand("push offset " + name.str()));
		AsmCode::addCode(new AsmCommand("call crt_printf"));
		AsmCode::addCode(new AsmCommand("add", ESP, size + 4));
	}
}

Node* EntireVariableNode::attach(Node* node, bool isCallable)
{
	if (isCallable) {
		this->isCallable = isCallable;
		this->parameters = node->children;
		this->parameterTypes = new vector<SymType*>;

		for (auto &child : *(this->parameters)) {
			this->parameterTypes->push_back(child->getType());
		}
	}

	this->children->push_back(node);

	return this;
}

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
