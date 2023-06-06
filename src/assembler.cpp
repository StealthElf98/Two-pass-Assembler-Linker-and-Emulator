#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>
#include "../inc/assembler.hpp"
#include "../inc/section.hpp"

Assembler::Assembler() {
    symTable.push_back(new Symbol(0, NOTYP, 0, "UND")); //first symbol 0 0 0 UND
    currentLocation = 0;
    currentSection = 0;
    pass = FIRST;
    stopAssembling = false;
    sections.push_back(new Section("UND"));
}

Assembler::~Assembler(){
    
}

void Assembler::assemble(std::vector<std::string>& allLines) {
    firstPass(allLines);
    secondPass(allLines);
    printSymbolTable();
    for(int i = 1; i < sections.size(); i++)
        sections[i]->printSection();
}

void Assembler::firstPass(std::vector<std::string>& allLines) {
    for(int i = 0; i < allLines.size(); i++) {
        if(!allLines[i].empty() && !stopAssembling) {
            checkLine(allLines[i], FIRST);
        }
    }
}

void Assembler::secondPass(std::vector<std::string>& allLines) {
    stopAssembling = false;
    for(int i = 0; i < allLines.size(); i++) {
        if(!allLines[i].empty() && !stopAssembling) {
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
            for(int i = 0; i < symbols.size(); i++) {
                currentLocation += 4;
            }
        } else if(directive == ".skip") {
            currentLocation += std::stoi(temp);
        } else if(directive == ".end") {
            currentLocation = 0;
            currentSection = 0;
            pass = SECOND;
            stopAssembling = true;
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
            for(int i = 0; i < symbols.size(); i++) {
                if(isNumber(symbols[i])) {
                    if(literalTooBig(symbols[i])) {
                        std::cout << "ERROR: LITERAL TOO BIG!" << std::endl;
                        exit(0);
                    }
                
                    if(isHex(symbols[i])) {
                        appendZeroToHex(symbols[i]);
                        sections[currentSection]->addFourBytes(symbols[i]);
                    } else {
                        std::stringstream s;
                        s << "0x";
                        s << std::hex << std::stoi(symbols[i]);
                        std::string temp;
                        s >> temp;
                        appendZeroToHex(temp);
                        sections[currentSection]->addFourBytes(temp);
                    }
                } else {
                    int index = symbolExists(symbols[i]); 
                    if(index == -1) {
                        sections[currentSection]->skip(4);
                        //DODATI SIMBOL U RELOKACIONU TABELU
                    } else {
                        std::string val = std::to_string(symTable[index]->getValue());
                        val = "0x" + val;
                        appendZeroToHex(val);
                        sections[currentSection]->addFourBytes(val);
                    }
                }
            }
        } else if(directive == ".skip") {
            sections[currentSection]->skip(std::stoi(symbols[0]));
        } else if(directive == ".end") {
            stopAssembling = true;
        } else {
            std::cout << "ERROR: Directive not found!" << std::endl;
        }
    }
}

void Assembler::checkInstruction(std::string line, Pass pass) {
    if(pass == FIRST) {
        currentLocation += 4;
    } else {
        std::string instruction;
        std::vector<std::string> params;
        std::string temp;
        line.erase(remove(line.begin(), line.end(), ','), line.end());
        std::stringstream ssin(line);
        ssin >> instruction;
        ssin >> temp;
        params.push_back(temp);
        while(ssin >> temp) {
            params.push_back(temp);
        }

        if(instruction == "halt") {
            sections[currentSection]->skip(4);
            stopAssembling = true;
        } else if(instruction == "int") {
            std::string temp = "10000000";
            sections[currentSection]->addFourBytes(temp);
        } else if(instruction == "iret") {

        } else if(instruction == "call") {

        } else if(instruction == "ret") {
            
        } else if(instruction == "jmp") {
            
        } else if(instruction == "beq") {
            
        } else if(instruction == "bne") {
            
        } else if(instruction == "bgt") {
            
        } else if(instruction == "push") {
            
        } else if(instruction == "pop") {
            
        } else if(instruction == "xchg") {
            
        } else if(instruction == "add") {
            
        } else if(instruction == "sub") {
            
        } else if(instruction == "mul") {
            
        } else if(instruction == "div") {
            
        } else if(instruction == "not") { 
            
        } else if(instruction == "and") {
            
        } else if(instruction == "or") {
            
        } else if(instruction == "xor") {
            
        } else if(instruction == "shl") {
            
        } else if(instruction == "shr") {
            
        } else if(instruction == "ld") {
            
        } else if(instruction == "st") {
            
        } else if(instruction == "csrrd") {
            
        } else if(instruction == "csrwr") {
            
        } else {
            
        }
        currentLocation += 4;
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

bool Assembler::isNumber(std::string s) {
    s.erase(std::remove(s.begin(), s.end(), '_'), s.end());
    s.erase(std::remove(s.begin(), s.end(), '-'), s.end());
    // bool res = !std::regex_match(s, std::regex("^[A-Za-z]+$"));
    // std::cout << s + ": " + (res ? "TRUE" : "FALSE") << std::endl;
    return !std::regex_match(s, std::regex("^[A-Za-z]+$"));
}

bool Assembler::literalTooBig(std::string num) {
    if (num.find('x') != std::string::npos) { 
        return num.length() >= 6;
    } else {
        return std::stoi(num) >= 4096;
    }
}

bool Assembler::isHex(std::string num) {
    return num.find("0x") != std::string::npos;
}

void Assembler::appendZeroToHex(std::string &num) {
    if(num.length() < 5) {
        num.insert (2, 10 - num.length() , '0');
    } 
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