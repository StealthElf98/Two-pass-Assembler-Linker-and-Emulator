#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
#include "../inc/instruction.hpp"

class Section {
private:
    std::string name;
    std::vector<Instruction* > addr;  
public:
    Section(std::string name);
    void addFourBytes(std::string instr);
    std::string getName();
    ~Section();
};

#endif