#include "Assembler.h"

#include <sstream>
#include <fstream>

using namespace std;

vector<AsmConstant*> AsmCode::const_section = {};
vector<AsmElement*> AsmCode::code_section = {};

string AsmCode::output_path = "";

map<double, string> AsmCode::double_consts = {};
map<string, int> AsmCode::variables = {};

AsmElement::AsmElement(string name) : name(name) {}
string AsmElement::print()
{
	return string();
}

void AsmCode::addCode(AsmElement* element)
{
	code_section.push_back(element);
}

void AsmCode::addConstant(AsmConstant* constant)
{
	const_section.push_back(constant);
}

int AsmCode::getOrCreateVariable(std::string name, int size)
{
	static int offset = 4;

	if (auto it = variables.find(name) != variables.end()) {
		return variables[name];
	}

	offset += size;
	variables[name] = offset;

	return -1;
}

std::string AsmCode::getOrCreateDoubleConstant(const double value)
{
	auto it = double_consts.find(value);
	if (it != double_consts.end()) {
		return std::string("__real@") + it->second;
	}

	std::string result;
	auto mask = uint64_t(15) << 60;
	const auto val = *reinterpret_cast<const uint64_t*>(&value);

	for (auto i = 0; i < sizeof(value) * 2; ++i) {
		const unsigned char c = static_cast<const unsigned char>((val & mask) >> (60 - i * 4));

		result += c + (c < 10 ? 48 : 87);
		mask >>= 4;
	}

	if (result[0] >= 97) {
		result.insert(result.begin(), '0');
	}

	double_consts[value] = result;
	std::string name = std::string("__real@") + result;
	AsmCode::addConstant(new AsmConstant(name, "dq", result + "r"));

	return name;
}

void AsmCode::print()
{
	ofstream fout(AsmCode::output_path);

	fout << "include d:\\masm32\\include\\masm32rt.inc" << endl << endl;

	fout << ".xmm" << endl;
	if (const_section.size()) {
		fout << ".const" << endl;
		for (auto it = const_section.begin(); it != const_section.end(); it++) {
			fout << (*it)->print() << endl;
		}
	}

	fout << ".code" << endl;
	for (auto it = code_section.begin(); it != code_section.end(); it++) {
		fout << (*it)->print() << endl;
	}

	fout << endl;
	fout << "start:" << endl;
	fout << "call __@function0" << endl;
	fout << "exit" << endl;
	fout << "end start";

	fout.close();
}

void AsmCode::set_output_path(std::string output_path)
{
	string filename = output_path.substr(0, output_path.find("."));

	AsmCode::output_path = filename + ".res";
}

std::string AsmCode::get_register_verbose_name(RegisterType reg)
{
	switch (reg) {
	case EAX:
		return "eax";

	case EBX:
		return "ebx";

	case ECX:
		return "ecx";

	case EDX:
		return "edx";

	case AL:
		return "al";

	case AH:
		return "ah";

	case EBP:
		return "ebp";

	case ESP:
		return "esp";

	case XMM0:
		return "xmm0";

	case XMM1:
		return "xmm1";
			
	default:
		return "";
	}
}

AsmLabel::AsmLabel(std::string name) : AsmElement(name) {}

string AsmLabel::print()
{
	return name + ":";
}

AsmCommand::AsmCommand(std::string name) : AsmElement(name), argc(0) {}

AsmCommand::AsmCommand(std::string name, int val) : AsmElement(name), argc(1)
{
	args[0] = val;
}

AsmCommand::AsmCommand(std::string name, RegisterType reg) : AsmElement(name), argc(0)
{
	this->name = name + " " + AsmCode::get_register_verbose_name(reg);
}

AsmCommand::AsmCommand(std::string name, Ptr *ptr) : AsmElement(name), argc(0)
{
	this->name = name +  " " + ptr->print();
}

AsmCommand::AsmCommand(std::string name, int lval, int rval) : AsmElement(name), argc(2)
{
	args[0] = lval;
	args[1] = rval;
}

AsmCommand::AsmCommand(std::string name, RegisterType reg, int val) : AsmElement(name), argc(1)
{
	this->name = name + " " + AsmCode::get_register_verbose_name(reg) + ",";
	args[0] = val;
}

AsmCommand::AsmCommand(std::string name, RegisterType reg, Ptr* ptr) : AsmElement(name), argc(0)
{
	this->name = name + " " + AsmCode::get_register_verbose_name(reg) + ", " + ptr->print();
}

AsmCommand::AsmCommand(std::string name, Ptr* ptr, RegisterType reg) : AsmElement(name), argc(0)
{
	this->name = name + " " + ptr->print() + ", " + AsmCode::get_register_verbose_name(reg);
}

AsmCommand::AsmCommand(std::string name, RegisterType from, RegisterType to) : AsmElement(name), argc(0)
{
	this->name = name + " " + AsmCode::get_register_verbose_name(from) + ", " + AsmCode::get_register_verbose_name(to);
}

string AsmCommand::print()
{
	ostringstream str;
	str << name;

	if (argc) {
		str << " " << args[0];
	}

	for (int i = 1; i < argc; ++i) {
		str << ", " << args[i];
	}

	return str.str();
}

AsmConstant::AsmConstant(std::string name, std::string dx, std::string content) : name(name), dx(dx), content(content) {}

std::string AsmConstant::print()
{
	return name + " " + dx + " " + content;
}

Ptr::Ptr(RegisterType reg) : target(AsmCode::get_register_verbose_name(reg)) {}

Ptr::Ptr(string target) : target(target) {}

std::string Ptr::print()
{
	return std::string();
}

DwordPtr::DwordPtr(RegisterType reg) : Ptr(reg) {}

DwordPtr::DwordPtr(string target) : Ptr(target) {}

std::string DwordPtr::print()
{
	return "dword ptr [" + this->target + "]";
}

QwordPtr::QwordPtr(RegisterType reg) : Ptr(reg) {}

QwordPtr::QwordPtr(string target) : Ptr(target) {}

std::string QwordPtr::print()
{
	return "qword ptr [" + this->target + "]";
}