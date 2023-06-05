#include <string>
#include <vector>
#include "../inc/section.hpp"
#include "../inc/instruction.hpp"

Section::Section(std::string n): name(n) {}

void Section::addFourBytes(std::string instr) {
    std::cout << "Inst: " + instr << std::endl;
    Instruction* i = new Instruction(instr); 
    addr.push_back(i);
}

std::string Section::getName() {
    return name;
}