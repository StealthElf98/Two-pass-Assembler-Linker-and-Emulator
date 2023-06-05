#ifndef INSTRUCTION_H_
#define INSTRUCTION_H_

#include <string>
#include <iostream>
#include <vector>

class Instruction {
private:
    std::vector<std::string> addr;
public:
    Instruction(std::string instr);
    ~Instruction();
};

#endif
