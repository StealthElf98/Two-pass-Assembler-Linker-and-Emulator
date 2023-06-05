#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include "../inc/symbol.hpp"

int Symbol::globalNum = 0;

Symbol::Symbol(int v, Type t, int s, std::string n): name(n), value(v), type(t), section(s) {
    num = globalNum++;
    size = 0;
    bind = LOC;
}

int Symbol::getNum() {
    return num;
}
int Symbol::getValue() {
    return value;
}
int Symbol::getSize() {
    return size;
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

void Symbol::setTypeToSection() {
    type = SCTN;
}