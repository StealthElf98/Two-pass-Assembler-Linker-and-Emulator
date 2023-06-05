#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include "../inc/assembler.hpp"
#include "../inc/section.hpp"

Assembler::Assembler() {
    symTable.push_back(new Symbol(0, NOTYP, 0, "UND")); //first symbol 0 0 0 UND
    currentLocation = 0;
    currentSection = 0;
    pass = FIRST;
}

Assembler::~Assembler(){
    
}

void Assembler::assemble(std::vector<std::string>& allLines) {
    firstPass(allLines);
    secondPass(allLines);
    printSymbolTable();
}

void Assembler::firstPass(std::vector<std::string>& allLines) {
    for(int i = 0; i < allLines.size(); i++) {
        if(!allLines[i].empty()) {
            checkLine(allLines[i], FIRST);
        }
    }
}

void Assembler::secondPass(std::vector<std::string>& allLines) {
    std::cout << "DRUGI PROLAZ "<< std::endl;
     for(int i = 0; i < allLines.size(); i++) {
        if(!allLines[i].empty()) {
            checkLine(allLines[i], SECOND);
        }
    }
}    

void Assembler::checkLine(std::string line, Pass pass) {
    if(pass == FIRST) {
        if(line[0] == '#') return;
        if(line[0] == '.') {                     // .section, .extern
            checkDirective(line, FIRST);
            return;
        }
        if(line.find(':') == line.size() - 1) {  // .labela
            std::string label = line.substr(0, line.find(':'));
            if(symbolExists(label) == -1) {
                addSymbolToTable(label, currentLocation, currentSection, LOC, false);
            }
        } else {                                // .sveOstalo
            checkInstruction(line, FIRST);
            return;
        }
    } else {
        if (line.find(':') != std::string::npos || line[0] == '#') {
            return;
        }
        else if(line[0] == '.') {                 // .section, .extern
            checkDirective(line, SECOND);
            return;
        } else {
            checkInstruction(line, SECOND);
            return;
        }
    }
} 

void Assembler::checkDirective(std::string line, Pass pass) {
    if(pass == FIRST) {
        if(line.size() == 0 || line.at(0) != '.')
            return;
        std::string directive;
        std::vector<std::string> symbols;
        std::string temp;
        line.erase(remove(line.begin(), line.end(), ','), line.end());
        std::stringstream ssin(line);
        ssin >> directive;
        ssin >> temp;
        symbols.push_back(temp);
        while(ssin >> temp) {
            symbols.push_back(temp);
        }

        if(directive == ".extern") {
            for(int i = 0; i < symbols.size(); i++) 
                if(symbolExists(temp) == -1) 
                     addSymbolToTable(symbols[i], currentLocation, 0, GLOB, false);
        } else if(directive == ".section") {
            currentSection++;
            currentLocation = 0;
            if(symbolExists(temp) == -1) 
                addSymbolToTable(temp, currentLocation, currentSection, LOC, true);
        } else if(directive == ".word") {
            for(int i = 0; i < symbols.size(); i++)
                currentLocation += 4;
        } else if(directive == ".skip") {
            currentLocation += std::stoi(temp);
        } else if(directive == ".end") {
            currentLocation = 0;
            currentSection = 0;
            pass = SECOND;
        } else {
            if(directive != ".global")
                std::cout << "ERROR: Directive not found!" << std::endl;
        }
    } else {
        if(line.size() == 0 || line.at(0) != '.')
            return;
        std::string directive;
        std::vector<std::string> symbols;
        std::string temp;
        line.erase(remove(line.begin(), line.end(), ','), line.end());
        std::stringstream ssin(line);
        ssin >> directive;
        ssin >> temp;
        symbols.push_back(temp);
        while(ssin >> temp) {
            symbols.push_back(temp);
        }

        if(directive == ".global") {
            int index = symbolExists(symbols[0]); 
            if(index == -1) {
                addSymbolToTable(symbols[0], currentLocation, currentSection, GLOB, false);
            } else {
                symTable[index]->setGlobal();
            }
        } else if(directive == ".extern") {
           
        } else if(directive == ".section") {
            currentLocation = 0;
            currentSection++;
            relocationTables.push_back(new RelocationTable(symbols[0]));
            sections.push_back(new Section(symbols[0]));
        } else if(directive == ".word") {
            
        } else if(directive == ".skip") {
            
        } else if(directive == ".end") {
           
        } else {
            std::cout << "ERROR: Directive not found!" << std::endl;
        }
    }
}

void Assembler::checkInstruction(std::string line, Pass pass) {
    if(pass == FIRST) {
        currentLocation += 4;
    } else {

    }
}

int Assembler::symbolExists(std::string symbolName) {
    for(int i = 0; i < symTable.size(); i++)
        if(symTable[i]->getName() == symbolName) 
            return i;
    
    return -1;
}

void Assembler::addSymbolToTable(std::string symbolName, int lc, int section, Bind bind, bool isSection) {
    Symbol* s = new Symbol(lc, NOTYP, section, symbolName);
    if(bind == GLOB)
        s->setGlobal();
    if(isSection) 
        s->setTypeToSection();
    symTable.push_back(s);
}

void Assembler::printSymbolTable() {
    std::cout << "\n";
    std::cout << "                                        SYMBOL TABLE                " << std::endl;
    std::cout << "    Name       |     Section     |      Type      |    Value    |    Location    |    Number " << std::endl;
    std::string fullString;
    for(int i = 0; i < symTable.size(); i++) {
        fullString.clear();
        std::string n = symTable[i]->getName();
        n.resize(15, ' ');
        fullString += n;
        int section = symTable[i]->getSection();
        fullString += "\t\t" + std::to_string(section);

        std::string type = ((symTable[i]->getType() == SCTN) ? "SCTN " : "NOTYP");
        fullString += "\t\t" + type;

        std::stringstream ss;
        int offset = symTable[i]->getValue();
        ss << "0x" << std::hex << offset;
        fullString += "\t\t" + ss.str();
        std::string bind  = ((symTable[i]->getBind() == GLOB) ? "GLOB" : "LOC ");
        fullString += "\t\t" + bind;

        int num = symTable[i]->getNum();
        fullString += "\t\t" + std::to_string(num);
        std::cout << fullString << std::endl;
    }
    std::cout << "\n";
}





















// Instructions Assembler::convertIntoInstruction(std::string instruction) {
//     if(instruction == "halt") return HALT;
//     if(instruction == "int") return INT;
//     if(instruction == "iret") return IRET;
//     if(instruction == "call") return CALL;
//     if(instruction == "ret") return RET;
//     if(instruction == "jmp") return JMP;
//     if(instruction == "beq") return BEQ;
//     if(instruction == "bne") return BNE;
//     if(instruction == "bgt") return BGT;
//     if(instruction == "push") return PUSH;
//     if(instruction == "pop") return POP;
//     if(instruction == "xchg") return XCHG;
//     if(instruction == "add") return ADD;
//     if(instruction == "sub") return SUB;
//     if(instruction == "mul") return MUL;
//     if(instruction == "div") return DIV;
//     if(instruction == "not") return NOT;
//     if(instruction == "and") return AND;
//     if(instruction == "or") return OR;
//     if(instruction == "xor") return XOR;
//     if(instruction == "shl") return SHL;
//     if(instruction == "shr") return SHR;
//     if(instruction == "ld") return LD;
//     if(instruction == "st") return ST;
//     if(instruction == "csrrd") return CSRRD;
//     if(instruction == "csrwr") return CSRWR;
//     return ERR_INS;
// }