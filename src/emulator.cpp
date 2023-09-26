#include <string>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <map>
#include <algorithm>
#include "../inc/emulator.hpp"

inline bool fileExists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

Emulator::Emulator() {
    for(int i = 0; i < 14; i++) {
        regs.push_back("00000000");
    }

    for(int i = 0; i < 3; i++) {
        csr.push_back("00000000");
    }
}

Emulator::~Emulator() {}

int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string outputFile;

    if(argc > 2) {
        std::cout << "ERROR: Too many arguments!" << std::endl;
        exit(0);
    }
    std::string programName = argv[1];
    if(programName.find(".hex") != std::string::npos) {
        std::string file= "../hex/" + programName;
        if(!fileExists(file)) {
            std::cout << "ERROR: Missing " + programName << std::endl;
            exit(0);
        }
    } else {
        std::cout << "ERROR! Wrong file type!" << std::endl;
        exit(0);
    }

    Emulator* emulator = new Emulator();
    emulator->readFile(programName);
    emulator->emulate();

   return 0;
} 

void Emulator::emulate() {
    pc = "40000000";
    bool done = false;
    while(!done) {
        std::string instruction = code[pc];
        std::string oc = instruction.substr(6, 2);
        std::string gprA (1, instruction[4]);
        std::string gprB (1, instruction[5]);
        std::string gprC (1, instruction[2]);
        std::string D = instruction[3] + instruction.substr(0, 2);

        movePC();

        if(oc == "00") {
            done == true;
            printRegisterValues();
            exit(0);
        } else if(oc == "10") {
            //push status na stek
            std::string newSP = calculateAddress("E", "00000000", "FFC");
            if(code.find(newSP) == code.end()) {
                // std::cout << "(INT PUSH STATUS)Ubacio adresu " + newSP + " u mem!" << std::endl;
                code[newSP] = convertFromLittleEndian(csr[0]);
                sp = newSP;
                // std::cout << "(INT PUSH STATUS)Na: " + newSP + " stavio status: " + code[newSP]<< std::endl;
            }
            newSP = calculateAddress("E", "00000000", "FFC");
            if(code.find(newSP) == code.end()) {
                // std::cout << "(INT PUSH PC)Ubacio adresu " + newSP + " u mem!" << std::endl;
                code[newSP] = convertFromLittleEndian(pc);
                sp = newSP;
                // std::cout << "(INT PUSH PC)Na: " + newSP + " stavio pc: " + code[newSP]<< std::endl;
            }
            csr[2] = "00000004";
            csr[0] = "00000000";
            pc = csr[1];
        } else if(oc == "20") {
            std::string newSP = calculateAddress("E", "00000000", D);
            if(code.find(newSP) == code.end()) {
                // std::cout << "(CALL PUSH)Ubacio adresu " + newSP + " u mem!" << std::endl;
                code[newSP] = convertFromLittleEndian(pc);
                sp = newSP;
            }

            pc = calculateAddress(gprA, gprB, D);
        } else if(oc == "21") {
            //pc na stek + sp - 4
            std::string newSP = calculateAddress("E", "00000000", "FFC");
            code[newSP] = convertFromLittleEndian(pc);
            sp = newSP;
            // std::cout << "(CALL PUSH)Na: " + newSP + " stavio PC: " + code[newSP]<< std::endl;

            std::string value = code[calculateAddress(gprA, gprB, D)];
            pc = convertFromLittleEndian(value);
            // std::cout << "(CALL) PC= " + pc + " u mem!" << std::endl;
        } else if(oc[0] == '3') {
            std::string regB = getValueFromReg(gprB);
            std::string regC = getValueFromReg(gprC);
            unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
            unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16); 

            // std::cout << "(B) regB=" + regB + " regC=" + regC << std::endl;
            if(oc[1] == '0') {
                pc = calculateAddress(gprA, "00000000", D);
            } else if(oc[1] == '1') {
                if(rgB == rgC) {
                    pc = calculateAddress(gprA, "00000000", D);
                    
                    // std::cout << "(BEQ) PC= " + pc << std::endl;
                }
            } else if(oc[1] == '2') {
                if(rgB != rgC) {
                    pc = calculateAddress(gprA, "00000000", D);
                }
            } else if(oc[1] == '3') {
                if(rgB > rgC) {
                    pc = calculateAddress(gprA, "00000000", D);
                }
            } else if(oc[1] == '8') {
                std::string temp = code[calculateAddress(gprA, "00000000", D)];
                pc = convertFromLittleEndian(temp);
            } else if(oc[1] == '9') {
                if(rgB == rgC) {
                    std::string temp = code[calculateAddress(gprA, "00000000", D)];
                    pc = convertFromLittleEndian(temp);
                }
            } else if(oc[1] == 'A') {
                if(rgB != rgC) {
                    std::string temp = code[calculateAddress(gprA, "00000000", D)];
                    pc = convertFromLittleEndian(temp);
                }
            } else if(oc[1] == 'B')  {
                if(rgB > rgC) {
                    std::string temp = code[calculateAddress(gprA, "00000000", D)];
                    pc = convertFromLittleEndian(temp);
                }
            }
        } else if(oc == "40") {
            std::string regB = getValueFromReg(gprB);
            std::string regC = getValueFromReg(gprC);

            setRegisterValue(gprB, regC);
            setRegisterValue(gprC, regB);
        } else if(oc[0] == '5') {
            unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
            unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16);
            unsigned int rgA = 0;

            if(oc[1] == '0') {
                rgA = rgB + rgC;
            } else if(oc[1] == '1') {
                rgA = rgB - rgC;
            } else if(oc[1] == '2') {
                rgA = rgB * rgC;
            } else if(oc[1] == '3') {
                rgA = rgB / rgC;
            }

            std::stringstream ss;
            ss << std::hex << rgA;
            std::string temp = ss.str();
            transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

            // std::cout << "Nakon operacije reg" + gprA +  "= " + temp << std::endl; 
            setRegisterValue(gprA, temp);
        } else if(oc[0] == '6') {
            int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
            int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16);
            int rgA = 0;

            if(oc[1] == '0') {
                rgA = ~rgB; 
            } else if(oc[1] == '1') {
                rgA = rgB & rgC;
            } else if(oc[1] == '2') {
                rgA = rgB | rgC;
            } else if(oc[1] == '3') {
                rgA = rgB ^ rgC;
            }

            std::stringstream ss;
            ss << std::hex << rgA;
            std::string temp = makeFourBytes(ss.str());
            transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

            setRegisterValue(gprA, temp);
        } else if(oc[0] == '7') {
            unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
            unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16);
            unsigned int rgA = 0;

            if(oc[1] == '0') {
                rgA = rgB >> rgC;
            } else if(oc[1] == '1') {
                rgA = rgB << rgC;
            }

            std::stringstream ss;
            ss << std::hex << rgA;
            std::string temp = makeFourBytes(ss.str());
            transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

            setRegisterValue(gprA, temp);
        } else if(oc[0] == '8') {
            std::string regA = getValueFromReg(gprA);
            std::string regB = getValueFromReg(gprB);
            std::string regC = getValueFromReg(gprC);
            
            if(oc[1] == '0') {
                // std::cout << "(STORE) Vrednosti reg: " + regA + " " + regB + " " + D << std::endl;
                std::string temp = calculateAddress(gprA, gprB, D);

                code[temp] = convertFromLittleEndian(regC);
                // std::cout << "(STORE) Sacuvao: " + regC + " na:" + temp << std::endl;
            } else if(oc[1] == '2') {
                std::string temp = code[calculateAddress(gprA, gprB, D)];
                temp = convertFromLittleEndian(temp);

                // std::cout << "(STORE) Prvi temp" + temp << std::endl;

                code[temp] = convertFromLittleEndian(makeFourBytes(regC));
                // std::cout << "(STORE) Sacuvao: " + regC + " na:" + temp << std::endl;
            } else if(oc[1] == '1') { //push
                std::string newSP = calculateAddress(gprA, "00000000", D);
                if(code.find(newSP) == code.end()) {
                    // std::cout << "(PUSH)Ubacio adresu " + newSP + " u mem!" << std::endl;
                    code[newSP] = "00000000";
                }
                sp = newSP;
                setRegisterValue(gprA, newSP);

                code[newSP] = convertFromLittleEndian(regC);
                // std::cout << "(PUSH)Adresa: " + newSP + " vrednost: " +  code[newSP] << std::endl;
            } 
        } else if(oc[0] == '9') {
            std::string regA = getValueFromReg(gprA);
            std::string regB = getValueFromReg(gprB);
            std::string regC = getValueFromReg(gprC);

            if(oc[1] == '0') {
                std::string csrValue = getValueFromCSR(gprB);
                setRegisterValue(gprA, csrValue);
            } else if(oc[1] == '1') {
                std::string value = calculateAddress("00000000", gprB, D);
                setRegisterValue(gprA, makeFourBytes(value));

                // std::cout << "(LOAD) reg" + gprA + " vrednost: " + getValueFromReg(gprA) << std::endl;
            } else if(oc[1] == '2') {
                std::string value = calculateAddress(gprB, gprC, D);
                value = convertFromLittleEndian(code[value]);
                if(gprA == "E") {
                    if(code.find(value) == code.end()) {
                        // std::cout << "Ubacio adresu " + value + " u mem!" << std::endl;
                        code[value] = "00000000";
                    }
                    setRegisterValue(gprA, value);
                } else {
                    // std::cout << "(LOAD) reg" + gprA + "<="<< value << std::endl;
                    setRegisterValue(gprA, value);
                }
            } else if(oc[1] == '3') { //pop
                if(code.find(sp) == code.end()) {
                    code[sp] = "00000000";
                }
                std::string value = convertFromLittleEndian(code[sp]);
                // std::cout << "Vrednost za pop: " + code[sp] << std::endl;
                setRegisterValue(gprA, value);
                // std::cout << "(POP)Izbacio: " + value << std::endl;
                
                value = calculateAddress("00000000", gprB, D);
                // std::cout << "(POP)SP= " + value + " posle pop-a" << std::endl;
                setRegisterValue(gprB, value);
                sp = value;
            } else if(oc[1] == '4') {
                setValueForCSR(gprA, regB);
            } else if(oc[1] == '5') {
                std::string regA = getValueFromCSR(gprA);
                std::string regB = getValueFromCSR(gprB);

                unsigned int rgA = std::stoul(getValueFromReg(gprA), nullptr, 16);
                unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16);
                unsigned int off = std::stoul(getValueFromReg(D), nullptr, 16);
                
                rgA = rgB | off;
                std::stringstream ss;
                ss << std::hex << rgA;
                std::string value = makeFourBytes(ss.str());
                setValueForCSR(gprA, value);
            } else if(oc[1] == '6') {
                std::string value = calculateAddress(gprB, gprC, D);
                setValueForCSR(gprA, convertFromLittleEndian(code[value]));
            } else if(oc[1] == '7') {
                std::string regB = getValueFromCSR(gprB);
                std::string value = convertFromLittleEndian(code[regB]);

                setValueForCSR(gprA, value);

                value = makeFourBytes(calculateAddress("00000000", regB, D));
                transform(value.begin(), value.end(), value.begin(), ::toupper);
                setRegisterValue(gprB, value);
            }   
        } else {

        }
    }
}

void Emulator::printRegisterValues() {
    std::cout << "Emulated processor executed halt instruction" << std::endl;
    std::cout <<"Emulated processor state:" << std::endl;
    for(int i = 0; i < regs.size(); i++) {
        if(i % 4 == 0) 
            std::cout << std::endl;
        if(i < 10)
            std::cout << " r" + std::to_string(i) + "=0x" + makeFourBytes(regs[i]) + " ";
        else 
            std::cout << "r" + std::to_string(i) + "=0x" + makeFourBytes(regs[i]) + " ";
    }
    std::cout << "r14=0x" + sp + " ";
    std::cout << "r15=0x" + pc + " " << std::endl;
}

std::string Emulator::makeFourBytes(std::string value) {
    if(value.length() < 8) {
        value.insert(0, 8 - value.length(), '0');
    }
    return value;
}

std::string Emulator::getValueFromCSR(std::string num) {
    if(num == "0") {
        return csr[0];
    } else if(num == "1") {
        return csr[1];
    } else {
        return csr[2];
    }
}

void Emulator::setValueForCSR(std::string num, std::string value) {
    if(num == "0") {
        csr[0] = value;
    } else if(num == "1") {
        csr[1] = value;
    } else {
        csr[2] = value;
    }
}

std::string Emulator::convertFromLittleEndian(std::string value) {
    return value.substr(6,2) + value.substr(4,2) + value.substr(2,2) + value.substr(0,2); 
}

std::string Emulator::calculateAddress(std::string gprA, std::string gprB, std::string D) {
    std::string sA = getValueFromReg(gprA);
    std::string sB = getValueFromReg(gprB);

    unsigned int vA = std::stoul(sA, nullptr, 16);
    unsigned int vB = std::stoul(sB, nullptr, 16);
    int vD = 0;
    if(D.length() == 3 && (D[0] == '8' || D[0] == '9' || D[0] == 'A' || D[0] == 'B' || D[0] == 'C' || D[0] == 'D' || D[0] == 'E' || D[0] == 'F')) {
        vD = std::stoul(D, nullptr, 16);
        vD -= 4096;
    } else {
        vD = std::stoul(D, nullptr, 16);
    }

    unsigned int result = vA + vB + vD;
    std::stringstream ss;
    ss << std::hex << result;
    std::string value;
    ss >> value;
    transform(value.begin(), value.end(), value.begin(), ::toupper);
    return value;
}

void Emulator::setRegisterValue(std::string reg, std::string value) {
    if(reg == "F" || reg == "f") {
        pc = value;
    } else if(reg == "E" || reg == "e") {
        sp = value;
    } else {
        int regNum = std::stoul(reg, nullptr, 16);
        regs[regNum] = value;
    }
}

std::string Emulator::getValueFromReg(std::string reg) {
    if(reg == "F" || reg == "f") {
        return pc;
    } else if(reg == "E" || reg == "e") {
        return sp;
    } else {
        int regNum = std::stoul(reg, nullptr, 16);
        transform(regs[regNum].begin(), regs[regNum].end(), regs[regNum].begin(), ::toupper);
        return regs[regNum];
    }
}

void Emulator::movePC() {
    std::stringstream ss;
    unsigned int decAddr = std::stoul(pc, nullptr, 16) + 4;
    ss << std::hex << decAddr;
    std::string newPC = ss.str();
    transform(newPC.begin(), newPC.end(), newPC.begin(), ::toupper);
    pc = newPC;
}


void Emulator::readFile(std::string fileName) {
    std::ifstream file("../hex/" + fileName);
    std::string line; 

    //34 22  
    while(std::getline(file, line)) {
        std::stringstream ss;
        if(!line.empty()) {
            std::string adr = line.substr(0, 8);
            transform(adr.begin(), adr.end(), adr.begin(), ::toupper);
            // std::cout << adr << std::endl;
            if(line.length() == 34) {
                std::string firstInstr = line.substr(10,11);
                firstInstr.erase(remove(firstInstr.begin(), firstInstr.end(), ' '), firstInstr.end());
                transform(firstInstr.begin(), firstInstr.end(), firstInstr.begin(), ::toupper);
                code[adr] = firstInstr;
                unsigned int decAddr = std::stoul(adr, nullptr, 16) + 4;
                ss << std::hex << decAddr;
                std::string fixedAddr = ss.str();
                transform(fixedAddr.begin(), fixedAddr.end(), fixedAddr.begin(), ::toupper);
                std::string secondInstr = line.substr(22,11);
                transform(secondInstr.begin(), secondInstr.end(), secondInstr.begin(), ::toupper);
                secondInstr.erase(remove(secondInstr.begin(), secondInstr.end(), ' '), secondInstr.end());
                code[fixedAddr] = secondInstr;
            } else if(line.length() == 22) {
                std::string firstInstr = line.substr(10,11);
                firstInstr.erase(remove(firstInstr.begin(), firstInstr.end(), ' '), firstInstr.end());
                transform(firstInstr.begin(), firstInstr.end(), firstInstr.begin(), ::toupper);
                code[adr] = firstInstr;
            }
        }
    }

    // for (auto const &pair: code) {
    //     std::cout << pair.first + ": " + pair.second << std::endl;
    // }

}
