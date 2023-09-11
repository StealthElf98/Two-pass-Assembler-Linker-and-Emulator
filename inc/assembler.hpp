#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include <string>
#include <vector>
#include <unordered_map>
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
    std::string name;
    int currentSectionId;
    std::vector<int> sectionIds;
    std::vector<Symbol*> symTable;
    std::unordered_map<int, std::vector<Relocation*>> relocationTables;
    // std::vector<RelocationTable*> relocationTables;
    std::vector<Section*> sections;
    Pass pass;
    bool stopAssembling;
    int currentLocation;
    int currentSection;
    std::vector<int> sectionSizes;
    void firstPass(std::vector<std::string>& allLines);
    void secondPass(std::vector<std::string>& allLines);
    void checkLine(std::string line, Pass pass);   
    void checkDirective(std::string line, Pass pass);
    void checkInstruction(std::string line, Pass pass);
    int symbolExists(std::string symbolName);
    void addSymbolToTable(std::string symbolName, int lc, int section, Bind bind, bool isSection);
    void printSymbolTable();
    bool isNumber(std::string s);
    bool literalTooBig(std::string num);
    bool isHex(std::string num);
    void appendZeroToHex(std::string &num);
    void appendZeroToD(std::string &num);
    std::string getRegisterNumber(std::string reg);
    std::string writeOffset();
    void printRelocationTables();
    int canFitIn(std::string num, int locationCounter);
    bool valueOfSymOK(int value, int locationCounter);
    int canJmpInD(std::string value);
    int canLoadInD(std::string value);
    std::string resolveRegister(std::string reg);
    std::string canSymFit(int value);
    int getCSR(std::string value);
    void writeInOutputFile();
public:
    void assemble(std::vector<std::string>& allLines);
    Assembler(std::string n);
    ~Assembler();
};

#endif