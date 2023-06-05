#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include <vector>
#include "../inc/relocationtable.hpp"
#include "../inc/symbol.hpp"
#include "../inc/section.hpp"

enum Instructions {
    HALT, INT, IRET, CALL, RET, JMP, BEQ, BNE, BGT, PUSH, POP, XCHG, ADD, SUB,
    MUL, DIV, NOT, AND, OR, XOR, SHL, SHR, LD, ST, CSRRD, CSRWR, ERR_INS
};

enum Directives {
    GLOBAL, EXTERN, SECTION, WORD, SKIP, END, ERR_DIR
};

enum Pass {
    FIRST, SECOND
};

class Assembler {
private:
    std::vector<Symbol*> symTable;
    std::vector<RelocationTable*> relocationTables;
    std::vector<Section*> sections;
    Pass pass;
    int currentLocation;
    int currentSection;
    void firstPass(std::vector<std::string>& allLines);
    void secondPass(std::vector<std::string>& allLines);
    void checkLine(std::string line, Pass pass);   
    void checkDirective(std::string line, Pass pass);
    void checkInstruction(std::string line, Pass pass);
    int symbolExists(std::string symbolName);
    void addSymbolToTable(std::string symbolName, int lc, int section, Bind bind, bool isSection);
    void printSymbolTable();
public:
    void assemble(std::vector<std::string>& allLines);
    Assembler();
    ~Assembler();
};

#endif