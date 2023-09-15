#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>
#include "../inc/instruction.hpp"

class Section {
private:
public:
    unsigned int startAddr = 0; // samo za linker
    std::string name;
    int sectionSize;
    std::string mergedAddresses = "";
    std::vector<std::string> addr;
    std::vector<std::string> pool;  
    std::vector<std::string> whichFile;
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
    int getPoolSize();
    unsigned int getSectionSizeForLinker();
    void relocate(std::string file, std::string value, int offset);
    void relocateForSection(std::string file, unsigned int value, int offset);
    void mergeAddresses();
    ~Section();
};

#endif