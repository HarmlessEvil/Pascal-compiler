#pragma once

#include <string>
#include <vector>

enum RegisterType {EAX, EBX, ECX, EDX, EBP, ESP};

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

	AsmCommand(std::string, int, int);
	AsmCommand(std::string, RegisterType, int);
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

	static void print();
	static void set_output_path(std::string);

	static std::string get_register_verbose_name(RegisterType);

private:
	static std::vector<AsmElement*> code_section;
	static std::vector<AsmConstant*> const_section;
	static std::string output_path;
};