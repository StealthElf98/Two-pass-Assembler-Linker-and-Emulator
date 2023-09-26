#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <regex>
#include <algorithm>
#include <fstream>
#include <unordered_map>
#include "../inc/assembler.hpp"
#include "../inc/section.hpp"

Assembler::Assembler(std::string n) {
    name = n;
    symTable.push_back(new Symbol(0, NOTYP, 0, "UND")); //first symbol 0 0 0 UND
    currentLocation = 0;
    currentSectionId = 0;
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

    writeInOutputFile();
    
    for(int i = 1; i < sections.size(); i++) {
        // std::cout << "Section " + sections[i]->getName() + " size: " + std::to_string(sections[i]->getSectionSize()) << std::endl;
        sections[i]->printSection();
    }

    printRelocationTables();

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
    if(line.empty()) return;
    if(pass == FIRST) {
        if(line[0] == '#') return;
        if(line[0] == '.') {                     // .section, .extern
            checkDirective(line, FIRST);
            return;
        }
        if(line.find(':') != std::string::npos) {  // .labela
            std::string label = line.substr(0, line.find(':'));
            if(symbolExists(label) == -1) {
                addSymbolToTable(label, currentLocation, currentSectionId, LOC, false);
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
            if(symbolExists(temp) == -1) 
                addSymbolToTable(temp, currentLocation, Symbol::globalNum, LOC, true);
            sectionIds.push_back(symTable.back()->getNum());
            currentSectionId = symTable.back()->getNum();
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
                    addSymbolToTable(symbols[i], currentLocation, currentSectionId, GLOB, false);
                } else {
                    symTable[index]->setGlobal();
                }
            }
        } else if(directive == ".extern") {
           
        } else if(directive == ".section") {
            currentLocation = 0;
            currentSection++;
            currentSectionId = sectionIds[currentSection-1];
            // std::cout << "Id trenutne sekcije: " + std::to_string(currentSectionId) << std::endl;
            relocationTables[currentSectionId] = std::vector<Relocation*>();
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
                            Relocation* rel = new Relocation(currentSectionId, currentLocation, symTable[index]->getNum());
                            relocationTables[currentSectionId].push_back(rel);                            
                            // relocationTables[currentSection]->addRelocation(rel);
                            sections[currentSection]->skip(4); 
                        } else {
                            Relocation* rel = new Relocation(currentSectionId, currentLocation, symTable[index]->getSection());
                            relocationTables[currentSectionId].push_back(rel);
                            // relocationTables[currentSection]->addRelocation(rel);
                            
                            std::stringstream s;
                            s << "0x" << std::hex << symTable[index]->getValue();
                            std::string temp;
                            s >> temp;
                            appendZeroToHex(temp);
                            sections[currentSection]->addFourBytes(temp);
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
        line.erase(remove(line.begin(), line.end(), ','), line.end());
        std::stringstream ssin(line);
        std::string instruction;
        ssin >> instruction;
        if(instruction == "ld") {
            std::string temp;
            ssin >> temp;
            if(temp.at(0) != '0' && temp.at(0) != '%' && temp.at(0) != '$' && temp.at(0) != '[') {
                currentLocation += 4;
            }
        } else if(instruction == "iret") {
                currentLocation += 4;
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

        // std::cout << line << std::endl;
        std::stringstream s;
        while(ssin >> temp) {
            params.push_back(temp);
        }

        if(instruction == "halt") {
            sections[currentSection]->skip(4);
        } else if(instruction == "int") {
            sections[currentSection]->addFourBytes("0x10000000");
        } else if(instruction == "iret") {
            //pop pc; pop status;
            sections[currentSection]->addFourBytes("0x960E0004");
            sections[currentSection]->addFourBytes("0x93FE0008");
            currentLocation += 4;
        } else if(instruction == "ret") {
            // 93(oc) F(pc) E(sp) 0(ne koristi se) 004(disp za sp)
            sections[currentSection]->addFourBytes("0x93FE0004");
        } else if(instruction == "call") {
            if(isNumber(params[0])) {
                std::stringstream ss;
                int literal = canFitIn(params[0], currentLocation); 
                if(literal != -1){
                    std::stringstream temp;
                    temp << "0x" << std::hex << literal;
                    temp >> params[0];
                    appendZeroToD(params[0]);

                    ss << "0x20000" << params[0].substr(2, 3);
                    sections[currentSection]->addFourBytes(ss.str());
                } else {
                    if(isHex(params[0])) {
                        appendZeroToHex(params[0]);
                        sections[currentSection]->addToPool(params[0]);
                    } else {
                        std::stringstream t;
                        t << "0x" <<  std::hex << std::stoi(params[0]);
                        std::string temp;
                        t >> temp;
                        appendZeroToHex(temp);
                        sections[currentSection]->addToPool(temp);
                    }

                    ss << "0x21F00" << writeOffset();
                    sections[currentSection]->addFourBytes(ss.str());
                }
            } else {
                std::stringstream t;
                int index = symbolExists(params[0]);
                if(index != -1) { // globalni simbol 21
                    if(symTable[index]->isGlobal()) {
                        sections[currentSection]->addToPool("0x00000000");
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getNum());
                        relocationTables[currentSectionId].push_back(rel);

                        std::stringstream ss;
                        ss << "0x21F00" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());
                    } else { // lokalini simbol
                        //if velicina i sekcija ok 20
                        if(valueOfSymOK(symTable[index]->getValue(), currentLocation) && symTable[index]->getSection() == currentSectionId) {
                            //pomeraj je value simbola - pc(lc)
                            int value = (symTable[index]->getValue() - currentLocation) >= 0 ? symTable[index]->getValue() - currentLocation - 4 : symTable[index]->getValue() - currentLocation;
                            t << std::hex << value;

                            std::string temp;
                            if(symTable[index]->getValue() - currentLocation < 0) {
                                temp = "0x" + t.str().substr(5, 3);
                            } else {
                                temp = "0x" + t.str();
                            }
                            appendZeroToD(temp);

                            std::stringstream ss;
                            ss << "0x20F00" << temp.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());
                        } else {
                            t << "0x" << std::hex << symTable[index]->getValue();
                            std::string temp = t.str();
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                            int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                            Relocation* rel = new Relocation(currentSectionId, lc,  symTable[index]->getSection());
                            relocationTables[currentSectionId].push_back(rel);

                            std::stringstream ss;
                            ss << "0x21000" << writeOffset();
                            sections[currentSection]->addFourBytes(ss.str());
                        }
                    }
                } else {
                    std::cout << "ERROR: COULDN'T FIND A SYMBOL!" << std::endl;
                }
            }         
        } else if(instruction == "jmp") {
            if(isNumber(params[0])) {
                std::stringstream ss;
                int literal = canJmpInD(params[0]); 
                if(literal != -1) {
                    // pc <= D 30
                    std::stringstream temp;
                    temp << "0x" << std::hex << literal;
                    temp >> params[0];
                    appendZeroToD(params[0]);

                    ss << "0x30000" << params[0].substr(2, 3);
                    sections[currentSection]->addFourBytes(ss.str()); 
                } else {
                    // pc <= pc + pom do bazena 38
                    if(isHex(params[0])) {
                        appendZeroToHex(params[0]);
                        sections[currentSection]->addToPool(params[0]);
                    } else {
                        std::stringstream t;
                        t << "0x" <<  std::hex << std::stoi(params[0]);
                        std::string temp;
                        t >> temp;
                        appendZeroToHex(temp);
                        sections[currentSection]->addToPool(temp);
                    }

                    ss << "0x38F00" << writeOffset();
                    sections[currentSection]->addFourBytes(ss.str()); 
                }
            } else {
                std::stringstream t;
                int index = symbolExists(params[0]);
                if(index != -1) {
                    if(symTable[index]->isGlobal()) {
                        sections[currentSection]->addToPool("0x00000000");
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getNum());
                        relocationTables[currentSectionId].push_back(rel);

                        std::stringstream ss;
                        ss << "0x38F00" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());    
                    } else {
                        if(valueOfSymOK(symTable[index]->getValue(), currentLocation) && symTable[index]->getSection() == currentSectionId) {
                            //pomeraj je value simbola - pc(lc)
                            int value = symTable[index]->getValue() - currentLocation - 4; 
                            t << std::hex << value;

                            std::string temp;
                            if(symTable[index]->getValue() - currentLocation - 4 < 0) {
                                temp = "0x" + t.str().substr(5, 3);
                            } else {
                                temp = "0x" + t.str();
                            }

                            appendZeroToD(temp);

                            std::stringstream ss;
                            ss << "0x30F00" << temp.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());
                        } else {
                            t << "0x" << std::hex << symTable[index]->getValue();
                            std::string temp = t.str();
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                            int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                            Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getSection());
                            relocationTables[currentSectionId].push_back(rel);

                            std::stringstream ss;
                            ss << "0x38F00" << writeOffset();
                            sections[currentSection]->addFourBytes(ss.str());
                        }
                    }
                }
            }
        } else if(instruction == "beq" || instruction == "bne" || instruction == "bgt") {
            std::vector<std::string> oc;
            if(instruction == "beq") {
                oc.push_back("31");
                oc.push_back("39");
            } else if(instruction == "bne") {
                oc.push_back("32");
                oc.push_back("3A");
            } else {
                oc.push_back("33");
                oc.push_back("3B");
            }
            std::string r1, r2;
            std::stringstream t;
            t << std::hex << std::stoi(getRegisterNumber(params[0]));
            r1 = t.str();
            t.str(std::string());
            t << std::hex << std::stoi(getRegisterNumber(params[1]));
            r2 = t.str();

            if(isNumber(params[2])) {
                std::stringstream ss;
                int literal = canJmpInD(params[2]); 
                if(literal != -1) {
                    // pc <= D 30
                    std::stringstream temp;
                    temp << "0x" << std::hex << literal;
                    temp >> params[2];
                    appendZeroToD(params[2]);

                    ss << "0x" << oc[0] <<"0" << r1 << r2 << params[2].substr(2, 3);
                    sections[currentSection]->addFourBytes(ss.str()); 
                } else {
                    // pc <= pc + pom do bazena 38
                    if(isHex(params[2])) {
                        appendZeroToHex(params[2]);
                        sections[currentSection]->addToPool(params[2]);
                    } else {
                        std::stringstream t;
                        t << "0x" <<  std::hex << std::stoi(params[2]);
                        std::string temp;
                        t >> temp;
                        appendZeroToHex(temp);
                        sections[currentSection]->addToPool(temp);
                    }

                    ss << "0x" << oc[1] << "F" << r1 << r2 << writeOffset();
                    sections[currentSection]->addFourBytes(ss.str()); 
                }
            } else {
                std::stringstream t;
                int index = symbolExists(params[2]);
                if(index != -1) {
                    if(symTable[index]->isGlobal()) {
                        sections[currentSection]->addToPool("0x00000000");
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getNum());
                        relocationTables[currentSectionId].push_back(rel);

                        std::stringstream ss;
                        ss << "0x" << oc[1] << "F" << r1 << r2 << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());    
                    } else {
                        if(valueOfSymOK(symTable[index]->getValue(), currentLocation) && symTable[index]->getSection() == currentSectionId) {
                            //pomeraj je value simbola - pc(lc)
                            // std::cout << " val: " + std::to_string(symTable[index]->getValue()) + " lc: " + std::to_string(currentLocation) << std::endl;
                            int value = symTable[index]->getValue() - currentLocation - 4; 
                            t << std::hex << value;

                            // std::cout << " VAL: " + t.str() << std::endl;
                            std::string temp;
                            if(symTable[index]->getValue() - currentLocation - 4 < 0) {
                                temp = "0x" + t.str().substr(5, 3);
                            } else {
                                temp = "0x" + t.str();
                            }

                            appendZeroToD(temp);


                            std::stringstream ss;
                            ss << "0x" << oc[0] << "F" << r1 << r2 << temp.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());
                        } else {
                            t << "0x" << std::hex << symTable[index]->getValue();
                            std::string temp = t.str();
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                            int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;
                            Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getSection());
                            relocationTables[currentSectionId].push_back(rel);

                            std::stringstream ss;
                            ss << "0x" << oc[1] << "F" << r1 << r2 << writeOffset();
                            sections[currentSection]->addFourBytes(ss.str());
                        }
                    }
                }
            }
        } else if(instruction == "push") {
            s << "0x81E0" << std::hex << std::stoi(getRegisterNumber(params[0])) << "FFC";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "pop") {
            s << "0x93" << std::hex << std::stoi(getRegisterNumber(params[0])) << "E0004";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "xchg") {
            s << "0x400" << std::hex << std::stoi(getRegisterNumber(params[0])) << std::hex << std::stoi(getRegisterNumber(params[1])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "add") {
            s << "0x50" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "sub") {
            s << "0x51" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "mul") {
            s << "0x52" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "div") {
            s << "0x53" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "not") {
            s << "0x60" << std::hex << std::stoi(getRegisterNumber(params[0])) << std::hex << std::stoi(getRegisterNumber(params[0])) << "0000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "and") {
            s << "0x61" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "or") {
            s << "0x62" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "xor") {
            s << "0x63" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "shl") {
            s << "0x70" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "shr") {
            s << "0x71" << std::hex << std::stoi(getRegisterNumber(params[1])) << std::hex << std::stoi(getRegisterNumber(params[1])) 
                << std::hex << std::stoi(getRegisterNumber(params[0])) << "000";
            sections[currentSection]->addFourBytes(s.str());
        } else if(instruction == "ld") {
            std::string reg;
            std::stringstream ss;
            std::stringstream t;
            
            // $ VRENOST SIMBOLA I LITERALA SE UCITAVA U REG
            if(params[0].at(0) == '$') {
                reg = resolveRegister(params[1]);
                std::string value = params[0].substr(1);
                if(isNumber(value)) {
                    int literal = canLoadInD(value); 
                    if(literal != -1) {
                        std::stringstream temp;
                        temp << "0x" << std::hex << literal;
                        temp >> value;
                        appendZeroToD(value);

                        ss << "0x91" << reg << "00" << value.substr(2, 3);
                        sections[currentSection]->addFourBytes(ss.str()); 
                    } else {
                        //literal veci od 4095 i 0xFFF
                        if(isHex(value)) {
                            appendZeroToHex(value);
                            sections[currentSection]->addToPool(value);
                        } else {
                            std::stringstream t;
                            t << "0x" <<  std::hex << std::stoi(value);
                            std::string temp;
                            t >> temp;
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                        }

                        ss << "0x92" << reg << "F0" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str()); 
                    }
                } else {
                    t.str(std::string());
                    int index = symbolExists(params[0].substr(1));
                    if(symTable[index]->isGlobal()) {
                        sections[currentSection]->addToPool("0x00000000");
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;

                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getNum());
                        relocationTables[currentSectionId].push_back(rel);

                        ss << "0x92" << reg << "F0" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());
                    } else {
                        if(valueOfSymOK(symTable[index]->getValue(), currentLocation) && symTable[index]->getSection() == currentSectionId) {
                            int value = symTable[index]->getValue() - currentLocation - 4; 
                            t << std::hex << value;

                            std::string temp;
                            if(symTable[index]->getValue() - currentLocation - 4 < 0) {
                                temp = "0x" + t.str().substr(5, 3);
                            } else {
                                temp = "0x" + t.str();
                            }

                            appendZeroToD(temp);

                            std::stringstream ss;
                            ss << "0x91" << reg << "F0" << temp.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());  
                        } 
                        else {
                            t << "0x" << std::hex << symTable[index]->getValue();
                            std::string temp = t.str();
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                            int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;

                            std::cout << sections[currentSection]->getSectionSize() << " 2LC= " << lc << std::endl;
                            Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getSection());
                            relocationTables[currentSectionId].push_back(rel);

                            ss << "0x92" << reg << "F0" << writeOffset();
                            sections[currentSection]->addFourBytes(ss.str());
                        }
                    }
                }
            } else if(params[0].at(0) == '%') {
                ss << "0x91" << resolveRegister(params[1]) << resolveRegister(params[0]) << "0000";
                sections[currentSection]->addFourBytes(ss.str());
            } else if(params[0].at(0) == '[' && params[1] != "+") {
                ss << "0x92" << resolveRegister(params[1]) << resolveRegister(params[0].substr(1)) << "0000";
                sections[currentSection]->addFourBytes(ss.str());
            } else if(params[0].at(0) == '[' && params[1] == "+") {                
                std::string reg = resolveRegister(params[0].substr(1));
                std::string litSim = params[2].substr(0, params[2].size()-1);
                std::string regUpis = resolveRegister(params[3]);

                if(isNumber(litSim)) {
                    if(canLoadInD(litSim) != -1) {
                        appendZeroToD(litSim);

                        ss << "0x92" << regUpis << reg << "0" << litSim.substr(2, 3);
                        sections[currentSection]->addFourBytes(ss.str());
                    } else {
                        std::cout << "ERROR: LITERAL TOO BIG!" << std::endl;
                        std::cout << line << std::endl;
                        exit(0);
                    }
                } else {
                    int index = symbolExists(litSim);
                    if(index != - 1) {
                        if(symTable[index]->isGlobal()) {
                            std::cout << "ERROR: SYMBOL VALUE UNKNOWN!" << std::endl;
                            std::cout << line << std::endl;
                            exit(0);
                        } 

                        std::string value = canSymFit(symTable[index]->getValue());
                        if(value != "") {
                            appendZeroToD(value);

                            ss << "0x92" << regUpis << reg << "0" << value.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());
                        } else {
                            std::cout << "ERROR: SYMBOL TOO BIG!" << std::endl;
                            std::cout << line << std::endl;
                            exit(0);
                        }
                    }
                }
            } else {
                reg = resolveRegister(params[1]);
                std::string value = params[0];
                if(isNumber(value)) {
                    int literal = canLoadInD(value); 
                    if(literal != -1) {
                        std::stringstream temp;
                        temp << "0x" << std::hex << literal;
                        temp >> value;
                        appendZeroToD(value);

                        ss << "0x92" << reg << "00" << value.substr(2, 3);
                        sections[currentSection]->addFourBytes(ss.str()); 
                    } else {
                        //literal veci od 4095 i 0xFFF
                        if(isHex(value)) {
                            appendZeroToHex(value);
                            sections[currentSection]->addToPool(value);
                        } else {
                            std::stringstream t;
                            t << "0x" <<  std::hex << std::stoi(value);
                            std::string temp;
                            t >> temp;
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                        }

                        ss << "0x92" << reg << "F0" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str()); 
                    }
                } else {
                    t.str(std::string());
                    int index = symbolExists(params[0]);
                    if(symTable[index]->isGlobal()) {
                        sections[currentSection]->addToPool("0x00000000");
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;

                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getNum());
                        relocationTables[currentSectionId].push_back(rel);

                        ss << "0x92" << reg << "F0" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());

                        ss.str(std::string());
                        ss << "0x92" << reg << reg << "0000";
                        sections[currentSection]->addFourBytes(ss.str());
                        currentLocation += 4; 
                    } else {
                        t << "0x" << std::hex << symTable[index]->getValue();
                        std::string temp = t.str();
                        appendZeroToHex(temp);
                        sections[currentSection]->addToPool(temp);
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;

                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getSection());
                        relocationTables[currentSectionId].push_back(rel);

                        ss << "0x92" << reg << "F0" << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());

                        ss.str(std::string());
                        ss << "0x92" << reg << reg << "0000"; 
                        sections[currentSection]->addFourBytes(ss.str());
                        currentLocation += 4;
                        // }
                    }
                }
            }

        } else if(instruction == "st") {
            std::stringstream ss;
            std::string reg = resolveRegister(params[0]);
            
            if(params[1].at(0) == '$') {
                std::cout << "ERROR: CAN'T LOAD VALUE INTO VALUE!" << std::endl;
                std::cout << line << std::endl;
                exit(0);
            } else if(params[1].at(0) == '%') {
                ss << "0x91" << resolveRegister(params[1]) << resolveRegister(params[0]) << "0000";
                sections[currentSection]->addFourBytes(ss.str());
                
            } else if(params[1].at(0) == '[' && params[1].at(params[1].length() - 1) == ']') {
                ss << "0x80" << resolveRegister(params[1].substr(1, params[1].length() - 2)) << "0" << reg << "000";
                sections[currentSection]->addFourBytes(ss.str());
            } else if(params[1].at(0) == '[' && params[2] == "+") {
                std::string reg1 = resolveRegister(params[1].substr(1));
                std::string litSim = params[3].substr(0, params[3].length()-1);

                if(isNumber(litSim)) {
                    if(canLoadInD(litSim) != -1) {
                        appendZeroToD(litSim);

                        ss << "0x80" << reg1 << "0" << reg << litSim.substr(2, 3);
                        sections[currentSection]->addFourBytes(ss.str());
                    } else {
                        std::cout << "ERROR: LITERAL TOO BIG!" << std::endl;
                        std::cout << line << std::endl;
                        exit(0);
                    }
                } else {
                    int index = symbolExists(litSim);
                    if(index != - 1) {
                        if(symTable[index]->isGlobal()) {
                            std::cout << "ERROR: SYMBOL VALUE UNKNOWN!" << std::endl;
                            std::cout << line << std::endl;
                            exit(0);
                        } 

                        std::string value = canSymFit(symTable[index]->getValue());
                        if(value != "") {
                            appendZeroToD(value);

                            ss << "0x80" << reg1 << "0" << reg << value.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());
                        } else {
                            std::cout << "ERROR: SYMBOL TOO BIG!" << std::endl;
                            std::cout << line << std::endl;
                            exit(0);
                        }
                    }
                }
            } else {
                reg = resolveRegister(params[0]);
                std::string value = params[1];
                if(isNumber(value)) {
                    int literal = canLoadInD(value); 
                    if(literal != -1) {
                        std::stringstream temp;
                        temp << "0x" << std::hex << literal;
                        temp >> value;
                        appendZeroToD(value);

                        ss << "0x80" << "00" << reg << value.substr(2, 3);
                        sections[currentSection]->addFourBytes(ss.str()); 
                    } else {
                        //literal veci od 4095 i 0xFFF
                        if(isHex(value)) {
                            appendZeroToHex(value);
                            sections[currentSection]->addToPool(value);
                        } else {
                            std::stringstream t;
                            t << "0x" <<  std::hex << std::stoi(value);
                            std::string temp;
                            t >> temp;
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                        }

                        ss << "0x82" << "F0" << reg  << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str()); 
                    }
                } else {
                    std::stringstream t;
                    t.str(std::string());
                    int index = symbolExists(params[1]);
                    if(symTable[index]->isGlobal()) {
                        sections[currentSection]->addToPool("0x00000000");
                        int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;

                        Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getNum());
                        relocationTables[currentSectionId].push_back(rel);

                        ss << "0x82" << "F0" << reg << writeOffset();
                        sections[currentSection]->addFourBytes(ss.str());
                    } else {
                        if(valueOfSymOK(symTable[index]->getValue(), currentLocation) && symTable[index]->getSection() == currentSectionId) {
                            int value = symTable[index]->getValue() - currentLocation - 4; 
                            t << std::hex << value;

                            std::string temp;
                            if(symTable[index]->getValue() - currentLocation - 4 < 0) {
                                temp = "0x" + t.str().substr(5, 3);
                            } else {
                                temp = "0x" + t.str();
                            }

                            appendZeroToD(temp);

                            std::stringstream ss;
                            ss << "0x80" << "F0" << reg  << temp.substr(2, 3);
                            sections[currentSection]->addFourBytes(ss.str());
                        } else {
                            t << "0x" << std::hex << symTable[index]->getValue();
                            std::string temp = t.str();
                            appendZeroToHex(temp);
                            sections[currentSection]->addToPool(temp);
                            int lc = sections[currentSection]->getSectionSize() + sections[currentSection]->getPoolSize() - 4;

                            Relocation* rel = new Relocation(currentSectionId, lc, symTable[index]->getSection());
                            relocationTables[currentSectionId].push_back(rel);

                            ss << "0x82" << "F0" << reg << writeOffset();
                            sections[currentSection]->addFourBytes(ss.str());
                        }
                    }
                }
            }
        } else if(instruction == "csrrd") {
            std::stringstream ss;
            std::string csrReg = std::to_string(getCSR(params[0]));
            std::string reg = getRegisterNumber(params[1]);

            ss << "0x90" << std::hex << std::stoi(reg) << csrReg << "0000";
            sections[currentSection]->addFourBytes(ss.str());
        } else if(instruction == "csrwr") {
            std::stringstream ss;
            std::string csrReg = std::to_string(getCSR(params[1]));
            std::string reg = getRegisterNumber(params[0]);

            std::stringstream t;
            ss << "0x94" << std::hex << std::stoi(reg) << csrReg << "0000";
            sections[currentSection]->addFourBytes(ss.str());
        } else {

            std::cout << "INSTRUCTION UNKNOWN1!" + params[0]<< std::endl;
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
    return !std::regex_match(s, std::regex("^[A-Za-z]+[0-9]*$"));
}

bool Assembler::literalTooBig(std::string num) {
    if (num.find('x') != std::string::npos) { 
        return num.length() > 10;
    } 
    return false;
}

int Assembler::canFitIn(std::string num, int locationCounter) {
    int n;
    // std::cout << "Pre: " + num << std::endl;
    if(num.find('x') != std::string::npos) {
        n = std::stoul(num, nullptr, 16);
    } else {
        n = std::stoi(num);
    }

    // std::cout << "Posle: " + std::to_string(n) << std::endl;
    if(abs(locationCounter - n) < 2047) {
        return n-locationCounter;
    }
    else
        return -1;
}

int Assembler::canJmpInD(std::string value) {
    int n;
    // std::cout << "Pre: " + num << std::endl;
    if(value.find('x') != std::string::npos) {
        n = std::stoul(value, nullptr, 16);
    } else {
        n = std::stoi(value);
    }

    // std::cout << "Posle: " + std::to_string(n) << std::endl;
    if(abs(n) < 2047) {
        return n;
    }
    else
        return -1;
}

int Assembler::canLoadInD(std::string value) {
    // std::cout << value << std::endl;

    int n = -1;
    // std::cout << "Pre: " + num << std::endl;
    if(value.find('x') != std::string::npos) {
        if(value.length() <= 5) 
            n = std::stoul(value, nullptr, 16);
        else 
            return -1;
    } else {
        n = std::stoi(value);
        if(n > 4095)
            return -1;
    }

    // std::cout << "Posle: " + std::to_string(n)<< std::endl;
    return n;
}

std::string Assembler::canSymFit(int value) {
    if(value <= 4095) {
        std::stringstream ss;
        ss << "0x" << std::hex << value;
        return ss.str();
    }
    
    return "";
}

int Assembler::getCSR(std::string value) {
    if(value == "%status") {
        return 0;
    } else if(value == "%handler") {
        return 1;
    } else {
        return 2;
    }
}

std::string Assembler::resolveRegister(std::string reg) {
    std::stringstream t;
    if (reg.substr(1) == "pc"){
        t << std::hex << 15;
    }
    else if (reg.substr(1) == "sp"){
        t << std::hex << 14;
    }
    else{
        t << std::hex << std::stoi(getRegisterNumber(reg));
    }

    return t.str();
}

bool Assembler::isHex(std::string num) {
    return num.find("0x") != std::string::npos;
}

void Assembler::appendZeroToHex(std::string &num) {
    if(num.length() < 10) {
        num.insert(2, 10 - num.length(), '0');
    }
}

void Assembler::appendZeroToD(std::string &num) {
    // std::cout << "Pre: " + num << std::endl;
    if(num.length() < 5) {
        num.insert(2, 5 - num.length(), '0');
        // std::cout << "Posle: " + num << std::endl;
    } 
}

std::string Assembler::getRegisterNumber(std::string reg) {
    if(reg.length() == 3) 
        return std::string{reg[2]};
    else {
        return reg.substr(2, 2);
    }
}

bool Assembler::valueOfSymOK(int value, int locationCounter) {
    // std::cout << locationCounter << std::endl;
    if((value - locationCounter) < 2047) {
        return true;
    }
    return false;
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

void Assembler::printRelocationTables() {
    for (const auto& pair : relocationTables) {
        std::cout << "rel.sec" + std::to_string(pair.first)<< std::endl;
        std::cout << "Section   Offset      Value" << std::endl;
        std::vector<Relocation*> relokacije = pair.second;
        for(int i = 0; i < relokacije.size(); i++) {
            std::cout << relokacije.at(i)->toString() << std::endl;
        }
        std::cout << "\n";
    }
}

std::string Assembler::getNameFromSymTable(std::string line) {
    std::stringstream ss(line);
    std::vector<std::string> params;
    std::string pom;
    for(int j = 0; j < 3; j++) {
        ss >> pom;
        params.push_back(pom);
    } 
   
    int n = std::stoi(pom);
    for(int i = 0; i < symTable.size(); i++) {
        if(symTable[i]->getNum() == n) 
            return symTable[i]->getName();
    } 
    
    return "";
}

void Assembler::writeInOutputFile() {
    std::ofstream outfile ("../o/" + name);

    outfile << name << std::endl;
    std::vector<std::string> sectionNames;
    sectionNames.push_back("UND");

    for(int i = 0; i<symTable.size(); i++) {
        if(symTable[i]->getType() == 1)
            sectionNames.push_back(symTable[i]->getName());
    }

    outfile << sections.size()-1 << std::endl; 
    for(int i = 1; i < sectionNames.size(); i++) {
        sections[i]->addr.insert( sections[i]->addr.end(), sections[i]->pool.begin(), sections[i]->pool.end());
        // std::cout << sections[i]->addr.size() << std::endl;
        // std::cout << sections[i]->getPoolSize() << std::endl;
        if(sections[i]->addr.size() > 0) {
            outfile << std::to_string(sectionIds[i-1]) + "\t" + sectionNames[i] + "\t" + std::to_string(sections[i]->addr.size()) + "\t";  
            for(int j = 0; j < sections[i]->addr.size(); j++)
                outfile << sections[i]->addr[j];  
            outfile << std::endl;
        }
    }

    outfile << std::endl;

    int tempSec = 0; 
    outfile << symTable.size()-1 << std::endl;
    for(int i = 1; i < symTable.size(); i++) {
        outfile << symTable[i]->getName() << "\t";
        if(symTable[i]->getType() == 1) 
            tempSec += 1;
        outfile << sectionNames.at(tempSec) << "\t";
        std::string tempString = (symTable[i]->getType() == 0) ? "NO" : "SCTN";
        outfile << tempString + "\t";
        outfile << symTable[i]->getValue() << "\t";
        tempString = (symTable[i]->getBind() == 0) ? "LOC" : "GLOB";
        outfile << tempString + "\t";
        outfile << symTable[i]->getNum() << std::endl;
    }

    outfile << std::endl;
    for (const auto& pair : relocationTables) {
        // std::cout << "VELICINA JE: " << pair.first + " " << pair.second.size() << std::endl;
        if(pair.second.size() == 0) {
            relocationTables.erase(pair.first);
            break;
        }
    }
    int size = relocationTables.size() == 0 ? 0 : relocationTables.size(); 
    outfile << size << std::endl;
    for (const auto& pair : relocationTables) {
        std::vector<Relocation*> relokacije = pair.second;
        std::string ime = symTable.at(pair.first)->getName();
        if(relokacije.size() > 0) {
            outfile << ime << std::endl;
            outfile << relokacije.size() << std::endl;
            for(int i = 0; i < relokacije.size(); i++) {
                outfile << relokacije.at(i)->toString();
                outfile << "\t" + getNameFromSymTable(relokacije.at(i)->toString()) << std::endl;
            }
            outfile << std::endl;
        }
    }

    outfile.close();
}
