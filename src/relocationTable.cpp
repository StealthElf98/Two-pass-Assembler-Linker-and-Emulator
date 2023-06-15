#include <string>
#include <vector>
#include "../inc/relocationTable.hpp"

RelocationTable::RelocationTable(std::string n): name(n) {}

void RelocationTable::addRelocation(Relocation* rel) {
    relocations.push_back(rel);
}

std::string RelocationTable::getName() {
    return name;
}