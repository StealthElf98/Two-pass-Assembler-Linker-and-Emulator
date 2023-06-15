#ifndef RELOCTABLE_H_
#define RELOCTABLE_H_

#include <string>
#include <vector>
#include "../inc/relocation.hpp"

class RelocationTable {
private:
    std::string name;
    std::vector<Relocation*> relocations;
public:
    RelocationTable(std::string name);
    ~RelocationTable();
    void addRelocation(Relocation* rel);
    std::string getName();
};

#endif