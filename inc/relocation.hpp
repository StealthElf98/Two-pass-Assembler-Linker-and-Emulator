#ifndef RELOCATION_H_
#define RELOCATION_H_

#include <string>
#include <vector>

class Relocation {
private:
    int sectionNum;
    int location;
    int val;
public:
    Relocation(int section, int offset, int num);
    ~Relocation();
    int getSectionNumber();
    void printRelocationTables();
    std::string toString();
};

#endif