#include <string>
#include <vector>
#include <iostream>
#include "../inc/relocationTable.hpp"

RelocationTable::RelocationTable(std::string name): tableName(name) {}

void RelocationTable::addRelocation(Relocation* rel) {
    relocations.push_back(rel);
    // std::cout << this->getName() << std::endl;
    std::cout << "Current size: " + std::to_string(relocations.size()) << std::endl;
}

std::string RelocationTable::getName() {
    return tableName;
}

int RelocationTable::getNumOfRelocations() {
    return relocations.size();
}


// void RelocationTable::printRelocationTable() {
//     std::cout << "Relokacije za sekciju: " + tableName + " " + std::to_string(relocations.size()) << std::endl;
//     for(int j = 0; j < getNumOfRelocations(); j++) {
//         std::cout << trelocations[j]->toString() << std::endl;
//     }
// }