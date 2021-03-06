#include "SyntaxAnalizer.h"

#include <algorithm>
#include <sstream>
#include <vector>

#include "../SemanticAnalyzer/SemanticAnalyzer.h"

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
		tokenizer->next();
		e = new SimpleExpressionNode(t, e, parseTerm());
		t = tokenizer->current();
	}

	return e;
}

Node* SyntaxAnalizer::parseSimpleConstantExpression()
{
	Node* e = parseConstantTerm();
	Token* t = tokenizer->current();

	while (t->getType() == ADDITION_OPERATOR) {
		tokenizer->next();
		e = new SimpleConstantExpressionNode(t, e, parseConstantTerm());
		t = tokenizer->current();
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
		program = new ProgramNode(t, heading, block);
	}
	else {
		program = parseBlock(false);
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
		e = new ProgramHeadingNode(t);
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
	Token* t = tokenizer->current();

	while (t->getType() == MULTIPLICATION_OPERATOR) {
		Token* asterisk = t;

		t = tokenizer->next();
		e = new TermNode(asterisk, e, parseFactor());
	}

	return e;
}

Node* SyntaxAnalizer::parseConstantTerm()
{
	Node* e = parseConstantFactor();
	Token* t = tokenizer->current();

	while (t->getType() == MULTIPLICATION_OPERATOR) {
		e = new ConstantTermNode(t, e, parseConstantFactor()); 
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
		Token* t = tokenizer->current();
		if (t->getType() == LBRACE) {
			return var->attach(parseFunctionDesignator());
		}
		else {
			return var;
		}
	}
	if (Node* literalNode = parseLiteral(t)) {
		return literalNode;
	}
	else if (type == LBRACE) {
		tokenizer->next();
		Node* e = parseExpression();
		Token* rbrace = tokenizer->current();
		if (!rbrace || rbrace->getType() != RBRACE) {
			throwError(rbrace->getPositionInString(), "Unclosed brace");
		}

		return e;
	}
	else if (type == ADDITION_OPERATOR) {
		return parseLiteral(tokenizer->next(), t->getSubType() == MINUS_OPERATOR);
	}

	if (t->getType() == ENDOFFILE) {
		throwError(t->getPositionInString(), "Unexpected EOF");
	}
	return NULL;
}

Node* SyntaxAnalizer::parseConstantFactor()
{
	Token* t = tokenizer->current();

	TokenType type = t->getType();
	Node* var = parseEntireConstantVariable();
	if (var) {
		Token* t = tokenizer->current();
		if (t->getType() == LBRACE) {
			return var->attach(parseConstantFunctionDesignator());
		}
		else {
			return var;
		}
	}
	if (type == INTEGER_LITERAL || type == FLOAT_LITERAL || type == STRING_LITERAL || type == NIL) {
		tokenizer->next();
		return new ConstantFactorNode(t);
	}
	else if (type == LBRACE) {
		Node* e = parseSimpleConstantExpression();
		Token* rbrace = tokenizer->current();
		if (!rbrace || rbrace->getType() != RBRACE) {
			throwError(rbrace->getPositionInString(), "Unclosed brace");
		}

		return e;
	}

	if (t->getType() == ENDOFFILE) {
		throwError(t->getPositionInString(), "Unexpected EOF");
	}
	return NULL;
}

Node* SyntaxAnalizer::parseBlock(bool has_heading)
{
	Token* t = new Token({-1, -1}, PROGRAM_BLOCK, "the program", NULL);
	vector<Node*>* declaration = parseDeclarationPart(has_heading);
	Node* statements = parseStatementPart();
	return new ProgramBlockNode(t, declaration, statements);
}

vector<Node*>* SyntaxAnalizer::parseDeclarationPart(bool has_heading)
{
	Token* t = has_heading ? tokenizer->next() : tokenizer->current();
	vector<Node*>* declarations = new vector<Node*>();
	
	while (t->getType() == DECLARATION_TYPE) {
		switch (t->getSubType())
		{
		case CONST_DEFINITION_KEYWORD:
			declarations->push_back(parseConstantDefinitionPart());
			break;

		case VAR_DEFINITION_KEYWORD:
			declarations->push_back(parseVarDefinitionPart());
			break;
		}

		t = tokenizer->current();
	}

	for (auto &decl : *declarations) {
		for (auto &type : *(decl->children)) {
			for (auto &var : *(type->children->at(1))->children) {
				var->generate();
			}
		}
	}

	return declarations;
}

Node* SyntaxAnalizer::parseStatementPart()
{
	Token* t = tokenizer->current();
	Node* statements = NULL;

	if (t->getType() != BEGIN_KEYWORD) {
		throwError(t->getPositionInString(), "Missing 'begin'");
	}

	statements = new StatementPartNode(t, parseStatementSequence());
	statements->generate();
	AsmCode::print();

	t = tokenizer->current();
	if (t->getType() == SEMICOLON) {
		t = tokenizer->next();
	}
	if (t->getType() != END_KEYWORD) {
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
	if (res) {
		return res;
	}
	else {
		return NULL;
	}
}

Node* SyntaxAnalizer::parseSimpleStatement()
{
	Node* statement = parseAssignmentStatement();
	if (!statement) {
		return NULL;
	}

	return statement;
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
		return variable;
	}

	tokenizer->next();
	Node* expression = parseExpression();
	assignment = new AssignmentNode(t, variable, expression);

	if (!variable->getType()->isCompatibleTo(expression->getType())) {
		throwError(t->getPositionInString(), "Incompatible types", "Semantic error");
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

	Node* field = parseFieldDesignator();
	if (field) {
		return variable->attach(field);
	}

	Token* t = tokenizer->current();
	if (t->getType() == LBRACE) {
		return variable->attach(parseProcedureStatement(), true);
	}

	return variable;
}

Node* SyntaxAnalizer::parseEntireVariable()
{
	Token* t = tokenizer->current();
	
	if (t->getType() == RESERVED_IDENTIFIER || t->getType() == UNRESERVED_IDENTIFIER) {
		return new EntireVariableNode(t);
	}
	else {
		return NULL;
	}
}

Node* SyntaxAnalizer::parseEntireConstantVariable()
{
	Token* t = tokenizer->current();

	if (t->getType() == RESERVED_IDENTIFIER || t->getType() == UNRESERVED_IDENTIFIER) {
		return new EntireConstantVariableNode(t);
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
		tokenizer->next();
		return new ExpressionNode(t, expression, parseSimpleExpression());
	}
	else {
		return expression;
	}
}

Node* SyntaxAnalizer::parseConstantExpression()
{
	Node* expression = parseSimpleConstantExpression();
	Token* t = tokenizer->current();

	if (t->getType() == RELATIONAL_OPERATOR) {
		tokenizer->next();
		return new ConstantExpressionNode(t, expression, parseSimpleExpression());
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

	statement = parseIfStatement();
	return statement;
}

Node* SyntaxAnalizer::parseCompoundStatement()
{
	Token* t = tokenizer->current();
	Node* statements = NULL;

	if (t->getType() != BEGIN_KEYWORD) {
		return NULL;
	}

	statements = new CompoundStatementNode(t, parseStatementSequence());

	t = tokenizer->current();
	if (t->getType() == SEMICOLON) {
		t = tokenizer->next();
	}
	if (t->getType() != END_KEYWORD) {
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
	tokenizer->next();
	expression = parseExpression();
	
	Token* t_do = tokenizer->current();
	Node* statement;
	if (t_do->getType() != DO_KEYWORD) {
		throwError(t_do->getPositionInString(), "Missing 'do' keyword after while statement");
		return NULL;
	}
	else {
		statement = parseStatement();
		return new WhileNode(t, expression, new Node(t_do, statement));
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
		tokenizer->next();
		expression = parseExpression();
		statements->push_back(new Node(t_until, expression));
		return new RepeatNode(t, statements);
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
		variable = new EntireVariableNode(t_var);

		Token* assignment = tokenizer->next();
		if (assignment->getType() != ASSIGNMENT_OPERATOR) {
			throwError(assignment->getPositionInString(), "Missing ':=' in loop definition");
		}

		tokenizer->next();
		Node* initial_value = new Node(assignment, variable, parseExpression());
		Token* direction = tokenizer->current();
		if (direction->getType() != FOR_DOWN_KEYWORD && direction->getType() != FOR_UP_KEYWORD) {
			throwError(direction->getPositionInString(), "Missing 'to'/'downto' in loop definition");
		}

		tokenizer->next();
		Node* to = new Node(direction, parseExpression());
		Token* t_do = tokenizer->current();
		if (t_do->getType() != DO_KEYWORD) {
			throwError(t_do->getPositionInString(), "Missing 'do' keyword");
		}

		return new ForNode(t, new vector<Node*>({ initial_value, to, new Node(t_do, parseStatement()) }));
	}
	else {
		throwError(t_var->getPositionInString(), "Missing loop variable");
		return NULL;
	}
}

Node* SyntaxAnalizer::parseIfStatement()
{
	Token* t = tokenizer->current();
	if (t->getType() != IF_HEADING) {
		return NULL;
	}

	tokenizer->next();
	Node* expression = parseExpression();
	Token* t_then = tokenizer->next();
	if (t_then->getType() != THEN_KEYWORD) {
		throwError(t_then->getPositionInString(), "Missing 'then' keyword");
	}

	Node* statement = parseStatement();
 	Token* t_else = tokenizer->current();
	if (t_else->getType() != ELSE_KEYWORD) {
		return new IfNode(t, expression, new Node(t_then, statement));
	}
	else {
		Node* else_statement = parseStatement();
		if (!else_statement) {
			throwError(tokenizer->current()->getPositionInString(), "Illegal expression");
		}
		return new IfNode(t, new vector<Node*>({ expression, new Node(t_then, statement), new Node(t_else, else_statement) }));
	}
}

Node* SyntaxAnalizer::parseComponentVariable()
{
	Node* var = parseIndexedVariable();
	if (var) {
		return var;
	}

	return parseFieldDesignator();
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

	return new IndicesOfVariableNode(t, expressions);
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
	vector<Node*>* parameters;

	parameters = parseActualParameterList();

	Token* t_brace = tokenizer->current();
	if (t_brace->getType() != RBRACE) {
		throwError(t_brace->getPositionInString(), "Missing ')' after procedure parameter list");
	}
	tokenizer->next();

	return new ProcedureStatementNode(t, parameters);
}

vector<Node*>* SyntaxAnalizer::parseActualParameterList()
{
	vector<Node*>* parameters = new vector<Node*>;

	parameters->push_back(parseActualParameter());
	Token* t = tokenizer->current();
	while (t->getType() == COMMA) {
		parameters->push_back(parseActualParameter());
		t = tokenizer->current();
	}

	return parameters;
}

vector<Node*>* SyntaxAnalizer::parseConstantActualParameterList()
{
	vector<Node*>* parameters = new vector<Node*>;

	parameters->push_back(parseConstantActualParameter());
	Token* t = tokenizer->current();
	while (t->getType() == COMMA) {
		parameters->push_back(parseConstantActualParameter());
		t = tokenizer->current();
	}

	return parameters;
}

Node* SyntaxAnalizer::parseActualParameter()
{
	Token* t = tokenizer->next();

	Node* parameter = parseExpression();
	if (parameter) {
		return parameter;
	}

	parameter = parseVariable();
	if (parameter) {
		return parameter;
	}

	return NULL;
}

Node* SyntaxAnalizer::parseConstantActualParameter()
{
	Token* t = tokenizer->next();

	Node* parameter = parseConstantExpression();
	if (parameter) {
		return parameter;
	}

	parameter = parseEntireConstantVariable();
	if (parameter) {
		return parameter;
	}

	return NULL;
}

Node* SyntaxAnalizer::parseFieldDesignator()
{
	Token* t = tokenizer->current();
	if (t->getType() != DOT) {
		return NULL;
	}

	Token* field_name = tokenizer->next();

	Node* variable = parseVariable();
	if (!variable) {
		throwError(t->getPositionInString(), "Expected identifier");
	}

	return new FieldDesignatorNode(t, variable);
}

Node* SyntaxAnalizer::parseFunctionDesignator()
{
	Token* t = tokenizer->current();
	vector<Node*>* parameters;

	parameters = parseActualParameterList();

	Token* t_brace = tokenizer->current();
	if (t_brace->getType() != RBRACE) {
		throwError(t_brace->getPositionInString(), "Missing ')' after function parameter list");
	}
	tokenizer->next();

	return new FunctionDesignatorParametersNode(t, parameters);
}

Node* SyntaxAnalizer::parseConstantFunctionDesignator()
{
	Token* t = tokenizer->current();
	vector<Node*>* parameters;

	parameters = parseConstantActualParameterList();

	Token* t_brace = tokenizer->current();
	if (t_brace->getType() != RBRACE) {
		throwError(t_brace->getPositionInString(), "Missing ')' after function parameter list");
	}
	tokenizer->next();

	return new ConstantFunctionDesignatorParametersNode(t, parameters);
}

Node* SyntaxAnalizer::parseConstantDefinitionPart()
{
	Token* t = tokenizer->current();
	Node* constant;
	vector<Node*>* constants = new vector<Node*>();

	while (constant = parseConstantDefinition()) {
		constants->push_back(constant);

		Token* semicolon = tokenizer->current();
		if (semicolon->getType() != SEMICOLON) {
			throwError(semicolon->getPositionInString(), "Missing ';' in constant definition");
		}
	}

	return new ConstantDefinitionPartNode(t, constants);
}

Node* SyntaxAnalizer::parseVarDefinitionPart()
{
	Token* t = tokenizer->current();
	Node* var;
	vector<Node*>* vars = new vector<Node*>();

	while (var = parseVarDefinition()) {
		Token* type = var->children->at(0)->get_token();
		vector<Node*>* var_names = var->children->at(1)->children;

		for (auto it = var_names->begin(); it != var_names->end(); it++) {
			Token* token = (*it)->get_token();
			SemanticAnalyzer::addVariable(token->getText(), type);
		}

		vars->push_back(var);

		Token* semicolon = tokenizer->current();
		if (semicolon->getType() != SEMICOLON) {
			throwError(semicolon->getPositionInString(), "Missing ';' in variables definition");
		}
	}
	
	return new Node(t, vars);
}

Node* SyntaxAnalizer::parseConstantDefinition()
{
	Token* t = tokenizer->next();
	Node* identifier = parseEntireVariable();
	if (!identifier) {
		return NULL;
	}
	
	Token* t_equals = tokenizer->next();
	if (t_equals->getType() != RELATIONAL_OPERATOR || t_equals->getSubType() != EQUALS_RELATION_OPERATOR) {
		throwError(t->getPositionInString(), "'=' expected");
	}

	tokenizer->next();
	return new Node(t_equals, identifier, parseConstantExpression());
}

Node* SyntaxAnalizer::parseVarDefinition()
{
	Token* t = tokenizer->next();
	Node* identifier = parseEntireVariable();
	vector<Node*>* identifiers = new vector<Node*>;
	if (!identifier) {
		return NULL;
	}
	identifiers->push_back(identifier);

	t = tokenizer->next();
	while (t->getType() == COMMA) {
		t = tokenizer->next();

		identifiers->push_back(parseEntireVariable());
		t = tokenizer->next();
	}

	if (t->getType() != COLON) {
		throwError(t->getPositionInString(), "Missing ':' in var definition");
	}

	Node* type = new Node();
	Token* colon = t;
	t = tokenizer->next();
	if (t->getType() == RESERVED_IDENTIFIER || t->getType() == RANGE || t->getType() == ARRAY_HEADING) {
		type = parseType();
	}
	
	tokenizer->next();
	return new Node(colon, type, new Node(colon, identifiers));
}

const unordered_set<string> SyntaxAnalizer::builtInTypes = {"integer", "real", "double", "long", "float"};

Node* SyntaxAnalizer::parseType()
{
	Token* t = tokenizer->current();
	TokenType t_type = t->getType();

	switch (t_type) {
	case RANGE:
		return new Node(t);

	case ARRAY_HEADING:
		return parseArrayDefinition();

	default:
		string type = t->getText();

		transform(type.begin(), type.end(), type.begin(), ::tolower);
		if (builtInTypes.find(type) == builtInTypes.end()) {
			throwError(t->getPositionInString(), "Type cannot be resolved!");
		}

		return new Node(t);
	}
}

Node* SyntaxAnalizer::parseArrayDefinition()
{
	Token* arr = tokenizer->current();
	Token* t = tokenizer->next();

	Token* brace = t;
	if (t->getType() != LSBRACE) {
		throwError(t->getPositionInString(), "Mssing '[' in array definition");
	}

	vector<Node*>* dimensions = new vector<Node*>;

	t = tokenizer->next();
	while (t->getType() == RANGE) {
		dimensions->push_back(new Node(t));

		t = tokenizer->next();
		if (t->getType() == RSBRACE) {
			t = tokenizer->next();
			break;
		}
		else if (t->getType() != COMMA) {
			throwError(t->getPositionInString(), "Missing ']' in array definition");
		}

		t = tokenizer->next();
	}

	Token* of = t;
	if (t->getType() != OF_KEYWORD) {
		throwError(t->getPositionInString(), "Missing 'of' keyword in array definition");
	}

	t = tokenizer->next();
	Node* type = parseType();

	static int counter = 0;
	counter++;
	ostringstream s;
	s << "array" << counter;

	int low, up = 100;
	string range = dimensions->at(0)->get_token()->getText();
	istringstream(range) >> low;
	stringstream ss;
	ss << low;
	istringstream(range.substr(range.find(ss.str()) + 3)) >> up;

	SemanticAnalyzer::addSymbol(s.str(), new SymTypeArray(SemanticAnalyzer::getIntegerType(), low, up));

	return new Node(arr, new Node(brace, dimensions), new Node(of, type));
}

Node* SyntaxAnalizer::parseLiteral(LexicalAnalyzer::Token* t, bool inverse)
{
	LexicalAnalyzer::TokenType type = t->getType();

	if (type == INTEGER_LITERAL || type == FLOAT_LITERAL || type == STRING_LITERAL || type == NIL) {
		tokenizer->next();

		if (inverse) {
			switch (type) {
			case INTEGER_LITERAL: {
				int* value = static_cast<int*>(t->getValue());
				int new_value = -*value;

				t->setValue(&new_value);

				ostringstream val;
				val << new_value;

				t->setText(val.str());

				break;
			}

			case FLOAT_LITERAL: {
				double* value = static_cast<double*>(t->getValue());
				double new_value = -*value;

				t->setValue(&new_value);

				ostringstream val;
				val << new_value;

				t->setText(val.str());

				break;
			}
			}
		}

		switch (type) {
		case INTEGER_LITERAL:
			return new FactorNode(t, SemanticAnalyzer::getIntegerType());

		case  FLOAT_LITERAL:
			return new FactorNode(t, SemanticAnalyzer::getFloatType());

		default:
			return NULL;
		}
	}
	else {
		return NULL;
	}
}

void SyntaxAnalizer::throwError(Token::StringCoord pos, char* text, string type)
{
	ostringstream s;
	s << type << " at (" << pos.row << ", " << pos.col << "): " << text;

	throw exception(s.str().c_str());
}
