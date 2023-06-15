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
    static int globalNum;
    int num;
    int value;
    int size;
    Type type;
    Bind bind;
    int section;
    std::string name;
public:
    Symbol(int value, Type type, int section, std::string name);
    ~Symbol();
    int getNum();
    int getValue();
    int getSize();
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