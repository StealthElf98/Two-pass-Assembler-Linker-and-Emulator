#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
#include "../inc/instruction.hpp"

class Section {
private:
    std::string name;
    std::vector<std::string> addr;  
public:
    Section(std::string name);
    void addFourBytes(std::string instr);
    void skip(int numOfBytes);
    std::string getName();
    void printSection();
    ~Section();
};

#endif