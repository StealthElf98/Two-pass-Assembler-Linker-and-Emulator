#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "../inc/symbol.hpp"

int Symbol::globalNum = 0;
int Symbol::globalNumForLinker = 1;

Symbol::Symbol(int v, Type t, int s, std::string n): name(n), value(v), type(t), section(s) {
    num = globalNum++;
    bind = LOC;
    valueForLinker = 0;
    sectionForLinker = "";
}

Symbol::Symbol(unsigned int value, std::string section, std::string name) {
    valueForLinker = value;
    sectionForLinker = section;
    this->name = name;
    num = globalNumForLinker++;
    this->value = 0;
    type = NOTYP;
    bind = LOC;
    this->section = 0;
}

int Symbol::getNum() {
    return num;
}
int Symbol::getValue() {
    return value;
}

Type Symbol::getType() {
    return type;
}
Bind Symbol::getBind() {
    return bind;
}
int Symbol::getSection() {
    return section;
}
std::string Symbol::getName() {
    return name;
}

std::string Symbol::toString() {
    std::stringstream ss;
    ss << name << " \t" << " \t " + section << " \t " + value << ((bind == GLOB) ? "GLOB" : "LOC") << " " + num;
    return ss.str();
}

void Symbol::setGlobal() {
    bind = GLOB;
}

bool Symbol::isGlobal() {
    return bind == GLOB;
}
void Symbol::setTypeToSection() {
    type = SCTN;
}