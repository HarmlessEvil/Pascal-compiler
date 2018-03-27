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

void Node::generate(bool isRvalue) {}

void Node::pushValueToStack()
{
}

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

void SimpleExpressionNode::generate(bool isRvalue)
{
	lhs->generate(true);
	rhs->generate(true);

	if (this->getType() == SemanticAnalyzer::getIntegerType()) {
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
	else if (this->getType() == SemanticAnalyzer::getFloatType()) {
		AsmCode::addCode(new AsmCommand("movsd", XMM1, new QwordPtr(ESP)));
		AsmCode::addCode(new AsmCommand("add", ESP, 8));
		AsmCode::addCode(new AsmCommand("movsd", XMM0, new QwordPtr(ESP)));

		switch (this->additionalOperator) {
		case PLUS_OPERATOR:
			AsmCode::addCode(new AsmCommand("addsd", XMM0, XMM1));
			break;

		case MINUS_OPERATOR:
			AsmCode::addCode(new AsmCommand("subsd", XMM0, XMM1));
			break;
		}

		AsmCode::addCode(new AsmCommand("movsd", new QwordPtr(ESP), XMM0));
	}
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

void TermNode::generate(bool isRvalue)
{
	lhs->generate(true);
	rhs->generate(true);

	if (this->getType() == SemanticAnalyzer::getIntegerType()) {
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
	else if (this->getType() == SemanticAnalyzer::getFloatType() || this->multiplicationOperator == FDIV_OPERATOR) {
		AsmCode::addCode(new AsmCommand("movsd", XMM1, new QwordPtr(ESP)));
		AsmCode::addCode(new AsmCommand("add", ESP, 8));
		AsmCode::addCode(new AsmCommand("movsd", XMM0, new QwordPtr(ESP)));

		switch (this->multiplicationOperator) {
		case MULTIPLICATION_OPERATOR:
			AsmCode::addCode(new AsmCommand("mulsd", XMM0, XMM1));
			break;

		case FDIV_OPERATOR:
			AsmCode::addCode(new AsmCommand("divsd", XMM0, XMM1));
			break;
		}

		AsmCode::addCode(new AsmCommand("movsd", new QwordPtr(ESP), XMM0));
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

void FactorNode::generate(bool isRvalue)
{
	switch (type) {
	case LexicalAnalyzer::INTEGER_LITERAL: {
		int *val = static_cast<int*>(this->token->getValue());
		AsmCode::addCode(new AsmCommand("push", *val));
		break;
	}

	case FLOAT_LITERAL: {
		double *val = static_cast<double*>(this->token->getValue());
		string name = AsmCode::getOrCreateDoubleConstant(*val);
		AsmCode::addCode(new AsmCommand("push", new DwordPtr(name + " + 4")));
		AsmCode::addCode(new AsmCommand("push", new DwordPtr(name)));
		break;
	}
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

void StatementPartNode::generate(bool isRvalue)
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

SymType* AssignmentNode::getType()
{
	return this->lhs->getType()->commonType(this->rhs->getType());
}

void AssignmentNode::generate(bool isRvalue)
{
	lhs->generate();
	rhs->generate(true);

	string size;
	if (this->getType() == SemanticAnalyzer::getIntegerType()) {
		AsmCode::addCode(new AsmCommand("pop", EAX));
		AsmCode::addCode(new AsmCommand("pop", EBX));

		AsmCode::addCode(new AsmCommand("mov dword ptr [ebx], eax"));
	}
	else if (this->getType() == SemanticAnalyzer::getFloatType()) {
		AsmCode::addCode(new AsmCommand("movsd xmm0, qword ptr [esp]"));
		AsmCode::addCode(new AsmCommand("add esp, 8"));
		AsmCode::addCode(new AsmCommand("pop ebx"));
		AsmCode::addCode(new AsmCommand("movsd qword ptr [ebx], xmm0"));
	}
}

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

void EntireVariableNode::generate(bool isRvalue)
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
			else {
				format << "37,100,32"; // %d
				size += 4;
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
			if (*it) {
				(*it)->generate(true);
			}
		}

		AsmCode::addCode(new AsmCommand("push offset " + name.str()));
		AsmCode::addCode(new AsmCommand("call crt_printf"));
		AsmCode::addCode(new AsmCommand("add", ESP, size + 4));
	}
	else {
		int offset = AsmCode::getOrCreateVariable(this->get_token()->getText(), this->getType()->getSize());

		if (offset > 0) {
			stringstream str;

			if (this->getType() == SemanticAnalyzer::getIntegerType()) {
				str << "lea eax, dword ptr [ebp - " << offset << "]";
				AsmCode::addCode(new AsmCommand(str.str()));
			}
			else if (this->getType() == SemanticAnalyzer::getFloatType()) {
				str << "lea eax, qword ptr [ebp - " << offset << "]";
				AsmCode::addCode(new AsmCommand(str.str()));
			}
			else if (this->getType() == SemanticAnalyzer::last_array) {
				string val = this->children->at(0)->children->at(0)->get_token()->getText();
				int num_val;
				istringstream(val) >> num_val;
				SymTypeArray* arr = static_cast<SymTypeArray*>(this->getType());

				int total_offset = arr->getElementSize() * num_val;
				str << "lea eax, dword ptr [ebp - " << offset + num_val * arr->getElementSize() << "]";
				AsmCode::addCode(new AsmCommand(str.str()));
			}

			AsmCode::addCode(new AsmCommand("push", EAX));

			if (isRvalue) {
				this->pushValueToStack();
			}
		}
	}
}

void EntireVariableNode::pushValueToStack()
{
	AsmCode::addCode(new AsmCommand("pop", EAX));

	if (this->getType() == SemanticAnalyzer::getIntegerType()) {
		AsmCode::addCode(new AsmCommand("push dword ptr [eax]"));
	}
	else if (this->getType() == SemanticAnalyzer::getFloatType()) {
		AsmCode::addCode(new AsmCommand("movsd xmm0, qword ptr [eax]"));
		AsmCode::addCode(new AsmCommand("sub esp, 8"));

		AsmCode::addCode(new AsmCommand("movsd qword ptr [esp], xmm0"));
	}
}

Node* EntireVariableNode::attach(Node* node, bool isCallable)
{
	if (isCallable) {
		this->isCallable = isCallable;
		this->parameters = node->children;
		this->parameterTypes = new vector<SymType*>;

		for (auto &child : *(this->parameters)) {
			if (child) {
				this->parameterTypes->push_back(child->getType());
			}
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

void ExpressionNode::generate(bool isRvalue)
{
	lexpr->generate(true);
	rexpr->generate(true);

	AsmCode::addCode(new AsmCommand("pop", EBX));
	AsmCode::addCode(new AsmCommand("pop", EAX));
	AsmCode::addCode(new AsmCommand("cmp", EAX, EBX));

	switch (this->relationalOperator) {
	case EQUALS_RELATION_OPERATOR:
		AsmCode::addCode(new AsmCommand("sete", AL));
		break;

	case NOT_EQUALS_RELATIONAL_OPERATOR:
		AsmCode::addCode(new AsmCommand("setne", AL));
		break;

	case GREATER_RELATIONAL_OPERATOR:
		AsmCode::addCode(new AsmCommand("setg", AL));
		break;

	case LESS_RELATIONAL_OPERATOR:
		AsmCode::addCode(new AsmCommand("setl", AL));
		break;

	case GREATER_OR_EQUALS_RELATIONAL_OPERATOR:
		AsmCode::addCode(new AsmCommand("setge", AL));
		break;

	case LESS_OR_EQUALS_RELATIONAL_OPERATOR:
		AsmCode::addCode(new AsmCommand("setle", AL));
		break;

	default:
		break;
	}
}

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

void IfNode::generate(bool isRvalue)
{
	this->condition->generate();
}

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

ArrayNode::ArrayNode(LexicalAnalyzer::Token* token, Node* dimensions, Node* type) : Node(token, dimensions, type)
{
	this->symType = SemanticAnalyzer::last_array;
}
