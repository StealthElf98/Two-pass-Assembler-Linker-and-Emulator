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

    for(int i = 1; i < sections.size(); i++) {
        std::cout << "Section " + sections[i]->getName() + " size: " + std::to_string(sections[i]->getSectionSize()) << std::endl;
        sections[i]->printSection();
    }
}

void Assembler::firstPass(std::vector<std::string>& allLines) {
    for(int i = 0; i < allLines.size(); i++) {
        if(!allLines[i].empty() && !stopAssembling) {
            checkLine(allLines[i], FIRST);
        } else 
            break;
    }
}

void Assembler::secondPass(std::vector<std::string>& allLines) {
    stopAssembling = false;
    for(int i = 0; i < allLines.size(); i++) {
        if(!allLines[i].empty() && !stopAssembling) {
            checkLine(allLines[i], SECOND);
        } else 
            break;
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
            sectionSizes.push_back(currentLocation);
            currentSection++;
            currentLocation = 0;
            currentLocation += sections[currentSection-1]->getEmptyPoolSize();
            if(symbolExists(temp) == -1) 
                addSymbolToTable(temp, currentLocation, currentSection, LOC, true);
        } else if(directive == ".word") {
            for(int i = 0; i < symbols.size(); i++) {
                currentLocation += 4;
            }
        } else if(directive == ".skip") {
            currentLocation += std::stoi(temp);
        } else if(directive == ".end") {
            sectionSizes.push_back(currentLocation);
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
            for(int i = 0; i < symbols.size(); i++) {
                int index = symbolExists(symbols[i]); 
                if(index == -1) {
                    addSymbolToTable(symbols[i], currentLocation, currentSection, GLOB, false);
                } else {
                    symTable[index]->setGlobal();
                }
            }
        } else if(directive == ".extern") {
           
        } else if(directive == ".section") {
            currentLocation = 0;
            currentSection++;
            relocationTables.push_back(new RelocationTable(symbols[0]));
            sections.push_back(new Section(symbols[0]));
            sections[currentSection]->setSectionSize(sectionSizes[currentSection]);
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
                        s << "0x" << std::hex << std::stoi(symbols[i]);
                        std::string temp;
                        s >> temp;
                        appendZeroToHex(temp);
                        sections[currentSection]->addFourBytes(temp);
                    }
                } else {
                    int index = symbolExists(symbols[i]); 
                    if(index == -1) {
                        std::cout << "Symbol unknown!" << std::endl;
                    } else {
                        if(symTable[index]->isGlobal()) {
                            sections[currentSection]->skip(4);
                            Relocation* rel = new Relocation(currentLocation, 0, symTable[index]->getNum());
                            relocationTables[currentSection-1]->addRelocation(rel);
                        } else {
                            std::stringstream val;
                            val << "0x" << std::hex << symTable[index]->getValue();
                            std::string v = val.str();
                            appendZeroToHex(v);
                            sections[currentSection]->addFourBytes(v);
                            Relocation* rel = new Relocation(currentLocation, 0, symTable[index]->getValue());
                            relocationTables[currentSection-1]->addRelocation(rel);
                        }
                    }
                }
                currentLocation += 4;
            }
        } else if(directive == ".skip") {
            sections[currentSection]->skip(std::stoi(symbols[0]));
            currentLocation += std::stoi(symbols[0]);
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
        std::string instruction;
        std::stringstream ssin(line);
        ssin >> instruction; 
        if(instruction == "call" || instruction == "jmp" || instruction == "beq" || instruction == "bne" || instruction == "bgt") {
            sections[currentSection]->increasePoolSize();
        }
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
        } else if(instruction == "int") {
            sections[currentSection]->addFourBytes("10000000");
        } else if(instruction == "iret") {
            
        } else if(instruction == "call") {
            if(isNumber(params[0])) {
                if(isHex(params[0])) {
                    appendZeroToHex(params[0]);
                    sections[currentSection]->addToPool(params[0]);
                } else {
                    std::stringstream s;
                    s << "0x" <<  std::hex << std::stoi(params[0]);
                    std::string temp;
                    s >> temp;
                    appendZeroToHex(temp);
                    sections[currentSection]->addToPool(temp);
                }
                std::stringstream ss;
                ss << "0x20000" << writeOffset();
                sections[currentSection]->addFourBytes(ss.str());
            } else {
                int index = symbolExists(params[0]);
                if(symTable[index]->isGlobal()) {
                    if(symTable[index]->getSection() != 0) {
                        // Symbol is global

                    } else {
                        // Symbol is extern

                    }
                } else {
                    // Adding symbol value to pool (to the end of current section)
                    std::stringstream s;
                    s << "0x" << std::hex << symTable[index]->getValue();
                    std::string temp = s.str();
                    appendZeroToHex(temp);
                    std::cout << "Value symbola: " + temp << std::endl;
                    sections[currentSection]->addToPool(temp);
                    int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                    std::cout << "Location counter symbola: " + std::to_string(lc) << std::endl;
                    relocationTables[currentSection]->addRelocation(new Relocation(lc, 0, symTable[index]->getValue()));
                }
                std::stringstream ss;
                ss << "0x21000" << writeOffset();
                std::cout << "Call with symbol: " + ss.str() << std::endl;
                sections[currentSection]->addFourBytes(ss.str());
            }         
        } else if(instruction == "ret") {
            // 93(oc) F(pc) E(sp) 0(ne koristi se) 004(disp za sp)
            sections[currentSection]->addFourBytes("0x93FE0004");
        } else if(instruction == "jmp") {

        } else if(instruction == "beq") {
            
        } else if(instruction == "bne") {
            
        } else if(instruction == "bgt") {
            
        } else if(instruction == "push") {
            std::stringstream s;
            s << "0x81E" << std::hex << std::stoi(getRegisterNumber(params[0])) << "0FFB";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "pop") {
            std::stringstream s;
            s << "0x93" << std::hex << std::stoi(getRegisterNumber(params[0])) << "E0004";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "xchg") {
            std::stringstream s;
            s << "0x400" << std::hex << std::stoi(getRegisterNumber(params[0])) << std::hex << std::stoi(getRegisterNumber(params[1])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "add") {
            std::stringstream s;
            s << "0x50" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "sub") {
            std::stringstream s;
            s << "0x51" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "mul") {
            std::stringstream s;
            s << "0x52" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "div") {
            std::stringstream s;
            s << "0x53" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "not") { 
            std::stringstream s;
            s << "0x60" << std::hex << std::stoi(getRegisterNumber(params[0])) << std::hex << std::stoi(getRegisterNumber(params[0])) << "0000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "and") {
            std::stringstream s;
            s << "0x61" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "or") {
            std::stringstream s;
            s << "0x62" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "xor") {
            std::stringstream s;
            s << "0x63" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "shl") {
            std::stringstream s;
            s << "0x70" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "shr") {
            std::stringstream s;
            s << "0x71" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
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
    return !std::regex_match(s, std::regex("^[A-Za-z]+$"));
}

bool Assembler::literalTooBig(std::string num) {
    if (num.find('x') != std::string::npos) { 
        return num.length() > 5;
    } else {
        return std::stoi(num) > 4095;
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

std::string Assembler::getRegisterNumber(std::string reg) {
    if(reg.length() == 3) 
        return std::string{reg[2]};
    else {
        return reg.substr(2, 2);
    }
}

std::string Assembler::writeOffset() {
    int offs = sections[currentSection]->getSectionSize() + (sections[currentSection]->getPoolSize() - 4) - currentLocation - 4;
    std::stringstream s;
    s << std::hex << offs;
    std::string temp = s.str();
    if(s.str().length() < 3) {
        temp.insert (0, 3 - temp.length(), '0');
    }
    return temp;
}