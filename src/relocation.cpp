#include <string>
#include <sstream>
#include <iostream>
#include "../inc/relocation.hpp"

Relocation::Relocation(int section, int offset, int num): sectionNum(section), location(offset), val(num) {}

Relocation::~Relocation(){}

int Relocation::getSectionNumber() {
    return sectionNum;
}

std::string Relocation::toString() {
    std::string relocationString;
    std::stringstream ss;
    ss << std::to_string(sectionNum) + "\t";
    ss << "   "  + std::to_string(location) + "\t";
    ss << "\t" + std::to_string(val);

    relocationString = ss.str();
    return relocationString;
}