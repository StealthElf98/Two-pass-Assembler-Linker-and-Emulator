#include <string>
#include <iostream>
#include "../inc/instruction.hpp"

Instruction::Instruction(std::string instr) { //0x12ABCDEF
    for(int i = 2; i <= 8; i+=2) 
        addr.push_back(instr.substr(i, 2));
    
    for(int i = 0; i < addr.size(); i++) 
        std::cout << "Addr: " + addr.at(i);
    // TREBA SMESTITI U LITTLE ENDIAN FORMATU JOS
}

Instruction::~Instruction() {

}
