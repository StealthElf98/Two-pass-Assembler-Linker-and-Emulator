#ifndef RELOCTABLE_H_
#define RELOCTABLE_H_

#include <string>
#include <vector>
#include "../inc/relocation.hpp"

class RelocationTable {
private:
public:
    std::string tableName;
    std::vector<Relocation*> relocations;
    RelocationTable(std::string name);
    ~RelocationTable();
    void addRelocation(Relocation* rel);
    std::string getName();
    int getNumOfRelocations();
    // void printRelocationTable();
};

#endif