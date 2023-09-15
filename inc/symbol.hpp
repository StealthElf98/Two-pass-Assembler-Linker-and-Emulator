#ifndef SYMBOL_H_
#define SYMBOL_H_
#include <cstring>
#include <iostream>
#include <vector>

enum Type {
    NOTYP, SCTN
};

enum Bind {
    LOC, GLOB
};

class Symbol {
private:
    int num;
    int value;
    Type type;
    Bind bind;
    int section;
    std::string name;
public:
    std::string sectionForLinker = "";
    unsigned int valueForLinker = 0;
    static int globalNum;
    static int globalNumForLinker;
    Symbol(int value, Type type, int section, std::string name);
    Symbol(unsigned int value, std::string section, std::string name);
    ~Symbol();
    int getNum();
    int getValue();
    Type getType();
    Bind getBind();
    int getSection();
    std::string getName();
    std::string toString();
    void setGlobal();
    void setTypeToSection();
    bool isGlobal();
};

#endif