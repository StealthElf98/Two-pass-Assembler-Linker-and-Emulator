#include <string>
#include <iostream>
#include "../inc/instruction.hpp"

Instruction::Instruction(std::string instr) { //0x12ABCDEF
    for(int i = 2; i <= 8; i+=2) 
        addr.push_back(instr.substr(i, 2));
    
    std::cout << "Addr: ";
    for(int i = 0; i < addr.size(); i++) 
        std::cout << addr.at(i);
    
    std::cout << std::endl;
    // TREBA SMESTITI U LITTLE ENDIAN FORMATU JOS
}

Instruction::~Instruction() {

}
