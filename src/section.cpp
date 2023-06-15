#include <string>
#include <vector>
#include "../inc/section.hpp"
#include "../inc/instruction.hpp"

Section::Section(std::string n): name(n) {}

void Section::addFourBytes(std::string instr) {
    std::vector<std::string> temp;
    for(int i = 2; i <= 8; i+=2) 
        temp.push_back(instr.substr(i, 2));
    for(int i = 3; i >= 0; i--) 
        addr.push_back(temp[i]);
}

std::string Section::getName() {
    return name;
}

void Section::skip(int numOfBytes) {
    for(int i = 0; i < numOfBytes; i++) 
        addr.push_back("00");
}

void Section::printSection() {
    std::cout << "SECTION " + getName() << std::endl;
    for(int i = 0; i < addr.size(); i++) {
        if(i % 8 == 0) 
            std::cout << std::endl;
        else if(i % 4 == 0) 
            std::cout << "  ";
        std::cout << addr.at(i) + " ";
    }

    std::cout << std::endl;
    std::cout << std::endl;
}

void Section::addOC(std::string oc) {

}
void Section::addThreeBytes(std::string opr) {

}