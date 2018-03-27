#pragma once

#include <map>
#include <string>
#include <vector>

enum RegisterType {EAX, EBX, ECX, EDX, AL, AH, EBP, ESP, XMM0, XMM1};

class Ptr {
public:
	Ptr(RegisterType);
	Ptr(std::string);

	virtual std::string print();

protected:
	std::string target;
};

class DwordPtr : public Ptr {
public:
	DwordPtr(RegisterType);
	DwordPtr(std::string);

	std::string print();
};

class QwordPtr : public Ptr {
public:
	QwordPtr(RegisterType);
	QwordPtr(std::string);

	std::string print();
};

class AsmElement {
public:
	AsmElement(std::string);

	virtual std::string print();

protected:
	std::string name;
};

class AsmLabel : public AsmElement {
public:
	AsmLabel(std::string);

	std::string print();
};

class AsmCommand : public AsmElement {
public:
	AsmCommand(std::string);

	AsmCommand(std::string, int);
	AsmCommand(std::string, RegisterType);
	AsmCommand(std::string, Ptr*);

	AsmCommand(std::string, int, int);
	AsmCommand(std::string, RegisterType, int);
	AsmCommand(std::string, RegisterType, Ptr*);
	AsmCommand(std::string, Ptr*, RegisterType);
	AsmCommand(std::string, RegisterType, RegisterType);

	std::string print();

private:
	int args[3];
	int argc;
};

class AsmConstant {
public:
	AsmConstant(std::string, std::string, std::string);

	std::string print();

private:
	std::string name;
	std::string dx;
	std::string content;
};

class AsmCode {
public:
	static void addCode(AsmElement*);
	static void addConstant(AsmConstant*);
	static int getOrCreateVariable(std::string, int);
	static std::string getOrCreateDoubleConstant(const double);

	static void print();
	static void set_output_path(std::string);

	static std::string get_register_verbose_name(RegisterType);

private:
	static std::vector<AsmElement*> code_section;
	static std::vector<AsmConstant*> const_section;
	static std::string output_path;

	static std::map<double, std::string> double_consts;
	static std::map<std::string, int> variables;
};