#include "SyntaxAnalizer.h"

#include <sstream>
 
using namespace LexicalAnalyzer;
using namespace std;

SyntaxAnalizer::SyntaxAnalizer(Tokenizer* tokenizer)
{
	this->tokenizer = tokenizer;
}

Node* SyntaxAnalizer::parseSimpleExpression()
{
	Node* e = parseTerm();
	Token* t = tokenizer->current();

	while (t->getType() == ADDITION_OPERATOR) {
		e = new Node(t, e, parseTerm());
	}

	return e;
}

Node* SyntaxAnalizer::parseProgram()
{
	Token* t = tokenizer->next();
	Node* program = NULL;

	if (t->getType() == PROGRAM_HEADING) {
		Node* heading = parseProgramHeading();
		Node* block = parseBlock();
		program = new Node(t, heading, block);
	}
	else {
		program = parseBlock();
	}

	t = tokenizer->next();
	if (t->getType() != DOT) {
		throwError(t->getPositionInString(), "Missing . after end");
	}
	
	return program;
}

Node * SyntaxAnalizer::parseProgramHeading()
{
	Node* e = NULL;
	Token* t = tokenizer->next();

	if (t->getType() != UNRESERVED_IDENTIFIER && t->getType() != RESERVED_IDENTIFIER) {
		throwError(t->getPositionInString(), "Missing program identifier");
	}
	else {
		e = new Node(t);
	}

	t = tokenizer->next();
	if (t->getType() != SEMICOLON) {
		throwError(t->getPositionInString(), "Missing semicolon");
	}
	return e;
}

Node* SyntaxAnalizer::parseTerm()
{
	Node* e = parseFactor();
	Token* t = tokenizer->next();

	while (t->getType() == MULTIPLICATION_OPERATOR) {
		e = new Node(t, e, parseFactor());
		t = tokenizer->next();
	}

	return e;
}

Node* SyntaxAnalizer::parseFactor()
{
	Token* t = tokenizer->current();
	
	TokenType type = t->getType();
	Node* var = parseVariable();
	if (var) {
		return var;
	}
	if (type == INTEGER || type == FLOAT || type == STRING || type == NIL) {
		return new Node(t);
	}
	else if (type == DELIMETER && *t == "(") {
		Node* e = parseSimpleExpression();
		Token* rbrace = tokenizer->current();
		if (!rbrace || rbrace->getType() != DELIMETER || *rbrace != ")") {
			throwError(rbrace->getPositionInString(), "Unclosed brace");
		}

		return e;
	}

	throwError(t->getPositionInString(), t->getType() == ENDOFFILE ? "Unexpected EOF" : "Unexpected token");
	return NULL;
}

Node* SyntaxAnalizer::parseBlock()
{
	Node* e = parseStatementPart();
	return e;
}

Node* SyntaxAnalizer::parseDeclarationPart()
{
	return nullptr;
}

Node* SyntaxAnalizer::parseStatementPart()
{
	Token* t = tokenizer->next();
	Node* statements = NULL;

	if (t->getType() != BEGIN_STATEMENT_PART) {
		throwError(t->getPositionInString(), "Missing 'begin'");
	}

	statements = new Node(t, parseStatementSequence());

	t = tokenizer->current();
	if (t->getType() == SEMICOLON) {
		t = tokenizer->next();
	}
	if (t->getType() != END_STATEMENT_PART) {
		throwError(t->getPositionInString(), "Missing 'end'");
	}

	return statements;
}

vector<Node*>* SyntaxAnalizer::parseStatementSequence()
{
	vector<Node*>* statements = new vector<Node*>();
	Token* t;

	statements->push_back(parseStatement());
	while ((t = tokenizer->current())->getType() == SEMICOLON) {
		statements->push_back(parseStatement());
	}

	return statements;
}

Node* SyntaxAnalizer::parseStatement()
{
	Node* res = parseSimpleStatement();
	if (res) {
		return res;
	}

	res = parseStructuredStatement();
	return res;
}

Node* SyntaxAnalizer::parseSimpleStatement()
{
	Node* statement = parseAssignmentStatement();
	if (statement) {
		return statement;
	}

	return parseProcedureStatement();
}

Node* SyntaxAnalizer::parseAssignmentStatement()
{
	Node* assignment = NULL;
	tokenizer->next();
	Node* variable = parseVariable();
	if (!variable) {
		return NULL;
	}
	
	Token* t = tokenizer->current();
	if (t->getType() != ASSIGNMENT_OPERATOR) {
		if (t->getType() == LBRACE) {
			parseProcedureStatement();
		}
		else {
			throwError(t->getPositionInString(), ":= expected");
		}
	}
	else {
		tokenizer->next();
		assignment = new Node(t, variable, parseExpression());
	}

	return assignment;
}

Node* SyntaxAnalizer::parseVariable()
{
	Node* variable = parseEntireVariable();
	if (!variable) {
		return NULL;
	}

	Node* indices = parseIndexedVariable();
	if (indices) {
		return variable->attach(indices);
	}
	return variable;
}

Node* SyntaxAnalizer::parseEntireVariable()
{
	Token* t = tokenizer->current();
	
	if (t->getType() == RESERVED_IDENTIFIER || t->getType() == UNRESERVED_IDENTIFIER) {
		return new Node(t);
	}
	else {
		return NULL;
	}
}

Node* SyntaxAnalizer::parseExpression()
{
	Node* expression = parseSimpleExpression();
	Token* t = tokenizer->current();

	if (t->getType() == RELATIONAL_OPERATOR) {
		return new Node(t, expression, parseSimpleExpression());
	}
	else {
		return expression;
	}
}

Node* SyntaxAnalizer::parseStructuredStatement()
{
	Node* statement = parseCompoundStatement();
	if (statement) {
		return statement;
	}

	statement = parseRepititiveStatement();
	if (statement) {
		return statement;
	}

	statement = parseIfStetement();
	return statement;
}

Node* SyntaxAnalizer::parseCompoundStatement()
{
	Token* t = tokenizer->current();
	Node* statements = NULL;

	if (t->getType() != BEGIN_STATEMENT_PART) {
		return NULL;
	}

	statements = new Node(t, parseStatementSequence());

	t = tokenizer->current();
	if (t->getType() == SEMICOLON) {
		t = tokenizer->next();
	}
	if (t->getType() != END_STATEMENT_PART) {
		throwError(t->getPositionInString(), "Missing 'end'");
	}

	tokenizer->next();

	return statements;
}

Node* SyntaxAnalizer::parseRepititiveStatement()
{
	Node* repititive_statement = parseWhileStatement();
	if (repititive_statement) {
		return repititive_statement;
	}

	repititive_statement = parseRepeatStatement();
	if (repititive_statement) {
		return repititive_statement;
	}

	return parseForStatement();
}

Node* SyntaxAnalizer::parseWhileStatement()
{
	Token* t = tokenizer->current();
	Node* expression;
	if (t->getType() != WHILE_HEADING) {
		return NULL;
	}
	expression = parseExpression();
	
	Token* t_do = tokenizer->current();
	Node* statement;
	if (t_do->getType() != DO_KEYWORD) {
		throwError(t_do->getPositionInString(), "Missing 'do' keyword after while statement");
		return NULL;
	}
	else {
		statement = parseStatement();
		return new Node(t, expression, new Node(t_do, statement));
	}
}

Node* SyntaxAnalizer::parseRepeatStatement()
{
	Token* t = tokenizer->current();
	vector<Node*>* statements;

	if (t->getType() != REPEAT_HEADING) {
		return NULL;
	}
	statements = parseStatementSequence();

	Token* t_until = tokenizer->current();
	Node* expression;
	if (t_until->getType() != UNTIL_KEYWORD) {
		throwError(t_until->getPositionInString(), "Missing 'until' keyword after repeat statement");
		return NULL;
	}
	else {
		expression = parseExpression();
		statements->push_back(new Node(t_until, expression));
		return new Node(t, statements);
	}
}

Node* SyntaxAnalizer::parseForStatement()
{
	Token* t = tokenizer->current();
	Node* variable;

	if (t->getType() != FOR_HEADING) {
		return NULL;
	}
	
	Token* t_var = tokenizer->next();
	if (t_var->getType() == RESERVED_IDENTIFIER || t_var->getType() == UNRESERVED_IDENTIFIER) {
		variable = new Node(t_var);

		Token* assignment = tokenizer->next();
		if (assignment->getType() != ASSIGNMENT_OPERATOR) {
			throwError(assignment->getPositionInString(), "Missing ':=' in loop definition");
		}

		Node* initial_value = new Node(assignment, variable, parseExpression());
		Token* direction = tokenizer->current();
		if (direction->getType() != FOR_DOWN_KEYWORD && direction->getType() != FOR_UP_KEYWORD) {
			throwError(direction->getPositionInString(), "Missing 'to'/'downto' in loop definition");
		}

		Node* to = new Node(direction, parseExpression());
		Token* t_do = tokenizer->current();
		if (t_do->getType() != DO_KEYWORD) {
			throwError(t_do->getPositionInString(), "Missing 'do' keyword");
		}

		return new Node(t, new vector<Node*>({ initial_value, to, new Node(t_do, parseStatement()) }));
	}
	else {
		throwError(t_var->getPositionInString(), "Missing loop variable");
		return NULL;
	}
}

Node* SyntaxAnalizer::parseIfStetement()
{
	Token* t = tokenizer->current();
	if (t->getType() != IF_HEADING) {
		return NULL;
	}

	Node* expression = parseExpression();
	Token* t_then = tokenizer->current();
	if (t_then->getType() != THEN_KEYWORD) {
		throwError(t_then->getPositionInString(), "Missing 'then' keyword");
	}

	Node* statement = parseStatement();
	Token* t_else = tokenizer->current();
	if (t_else->getType() != ELSE_KEYWORD) {
		return new Node(t, expression, new Node(t_then, statement));
	}
	else {
		Node* else_statement = parseStatement();
		if (!else_statement) {
			throwError(tokenizer->current()->getPositionInString(), "Illegal expression");
		}
		return new Node(t, new vector<Node*>({ expression, new Node(t_then, statement), new Node(t_else, else_statement) }));
	}
}

Node* SyntaxAnalizer::parseComponentVariable()
{
	return parseIndexedVariable();
}

Node* SyntaxAnalizer::parseIndexedVariable()
{
	Token* t = tokenizer->next();
	if (t->getType() != LSBRACE) {
		return NULL;
	}

	vector<Node*>* expressions = parseExpressionList();

	Token* t_brace = tokenizer->current();
	if (t_brace->getType() != RSBRACE) {
		throwError(t_brace->getPositionInString(), "Expected ']'");
	}
	tokenizer->next();

	return new Node(t, expressions);
}

vector<Node*>* SyntaxAnalizer::parseExpressionList()
{
	vector<Node*>* expressions = new vector<Node*>();

	tokenizer->next();
	expressions->push_back(parseExpression());
	
	Token* t = tokenizer->current();
	while (t->getType() == COMMA) {
		t = tokenizer->next();
		expressions->push_back(parseExpression());
	}

	return expressions;
}

Node* SyntaxAnalizer::parseProcedureStatement()
{
	Token* t = tokenizer->current();
	vector<Node*>* parameters = new vector<Node*>();

	parameters = parseActualParameterList();

	return nullptr;
}

vector<Node*>* SyntaxAnalizer::parseActualParameterList()
{
	vector<Node*>* parameters = new vector<Node*>;

	parameters->push_back(parseActualParameter());

	return parameters;
}

Node* SyntaxAnalizer::parseActualParameter()
{
	Token* t = tokenizer->current();

	Node* parameter = parseExpression();
	if (parameter) {
		return parameter;
	}

	parameter = parseVariable();
	if (parameter) {
		return parameter;
	}

	if (t->getType() == UNRESERVED_IDENTIFIER || t->getType() == RESERVED_IDENTIFIER) {
		return new Node(t);
	}

	return NULL;
}

void SyntaxAnalizer::throwError(Token::StringCoord pos,  char* text)
{
	ostringstream s;
	s << "Syntax error at (" << pos.row << ", " << pos.col << "): " << text;

	throw exception(s.str().c_str());
}
