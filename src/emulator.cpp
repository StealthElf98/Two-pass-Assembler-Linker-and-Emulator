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
    for(int i = 0; i < 13; i++) {
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
        std::string file= "../.hex/" + programName;
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
    int count = 0;
    pc = "40000000";

    while(pc != "halt") {
        std::string instruction = code[pc];
        std::string oc = instruction.substr(6, 2);
        std::string gprA (1, instruction[4]);
        std::string gprB (1, instruction[5]);
        std::string gprC (1, instruction[2]);
        std::string D = instruction[3] + instruction.substr(0, 2);

        movePC();

        if(oc == "00") {
            pc = "halt";
        } else if(oc == "10") {
            // stack.push_back(csr[0]);
            // stack.push_back(pc);
            // csr[2] = "00000004";
            // //nesto se uradi sa statusom crs[0]
            // pc = csr[1];
        } else if(oc == "20") {
            stack.push_back(pc);
            pc = calculateAddress(gprA, gprB, D);
        } else if(oc == "21") {
            stack.push_back(pc);
            std::string temp = code[calculateAddress(gprA, gprB, D)];
            pc = convertFromLittleEndian(temp);
        } else if(oc[0] == '3') {
            std::string regB = getValueFromReg(gprB);
            std::string regC = getValueFromReg(gprC);
            if(oc[1] == '0') {
                pc = calculateAddress(gprA, "00000000", D);
            } else if(oc[1] == '1') {
                if(regB == regC) {
                    pc = calculateAddress(gprA, "00000000", D);
                }
            } else if(oc[1] == '2') {
                if(regB != regC) {
                    pc = calculateAddress(gprA, "00000000", D);
                }
            } else if(oc[1] == '3') {
                unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
                unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16); 
                if(rgB > rgC) {
                    pc = calculateAddress(gprA, "00000000", D);
                }
            } else if(oc[1] == '8') {
                std::string temp = code[calculateAddress(gprA, "00000000", D)];
                pc = convertFromLittleEndian(temp);
            } else if(oc[1] == '9') {
                if(regB == regC) {
                    std::string temp = code[calculateAddress(gprA, "00000000", D)];
                    pc = convertFromLittleEndian(temp);
                }
            } else if(oc[1] == 'A') {
                if(regB != regC) {
                    std::string temp = code[calculateAddress(gprA, "00000000", D)];
                    pc = convertFromLittleEndian(temp);
                }
            } else if(oc[1] == 'B')  {
                unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
                unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16);
                if(rgB > rgC) {
                    std::string temp = code[calculateAddress(gprA, "00000000", D)];
                    pc = convertFromLittleEndian(temp);
                }
            }
        } else if(oc == "40") {
            int indexB = std::stoul(gprB, nullptr, 16);
            int indexC = std::stoul(gprC, nullptr, 16);
            std::string regB = getValueFromReg(gprB);
            std::string regC = getValueFromReg(gprC);

            regs[indexB] = regC;
            regs[indexC] = regB;
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

            setRegisterValue(gprA, temp);
        } else if(oc[0] == '6') {
            unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
            unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16);
            unsigned int rgA = 0;

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
            std::string temp = ss.str();
            transform(temp.begin(), temp.end(), temp.begin(), ::toupper);

            setRegisterValue(gprA, temp);
        } else if(oc[0] == '7') {
            unsigned int rgB = std::stoul(getValueFromReg(gprB), nullptr, 16); 
            unsigned int rgC = std::stoul(getValueFromReg(gprC), nullptr, 16);
            unsigned int rgA = 0;

            if(oc[1] == '0') {

            } else if(oc[1] == '1') {

            }
        } else if(oc[0] == '8') {
            
            if(oc[1] == '0') {

            } else if(oc[1] == '2') {

            } else if(oc[1] == '1') {

            } 
        } else if(oc[0] == '9') {
            
            if(oc[1] == '0') {

            } else if(oc[1] == '1') {

            } else if(oc[1] == '2') {

            } else if(oc[1] == '3') {

            } else if(oc[1] == '4') {

            } else if(oc[1] == '5') {

            } else if(oc[1] == '6') {

            } else if(oc[1] == '7') {

            }
        }

        std::cout << "PC=" + pc << std::endl;   
        count++;
        if(count == 50)
            break;
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
    std::ifstream file("../.hex/" + fileName);
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