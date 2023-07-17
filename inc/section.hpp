#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
#include "../inc/instruction.hpp"

class Section {
private:
    std::string name;
    std::vector<std::string> addr;
    std::vector<std::string> pool;  
    int sectionSize;
    int poolSize;
public:
    Section(std::string name);
    void addFourBytes(std::string instr);
    void skip(int numOfBytes);
    std::string getName();
    void printSection();
    void addOC(std::string oc);
    void addThreeBytes(std::string opr);
    void setSectionSize(int n);
    int getSectionSize();
    void addToPool(std::string val);
    void increasePoolSize();
    int getEmptyPoolSize();
    int getPoolSize();
    ~Section();
};

#endif