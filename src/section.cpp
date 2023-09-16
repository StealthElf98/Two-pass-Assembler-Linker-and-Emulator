#include <string>
#include <vector>
#include <sstream>
#include "../inc/section.hpp"
#include "../inc/instruction.hpp"

Section::Section(std::string n): name(n), sectionSize(0) {}

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

void Section::setSectionSize(int n) {
    sectionSize = n;
}

void Section::addToPool(std::string val) {
    std::vector<std::string> temp;
    for(int i = 2; i <= 8; i+=2) 
        temp.push_back(val.substr(i, 2));
    for(int i = 3; i >= 0; i--) 
        pool.push_back(temp[i]);
}

int Section::getSectionSize() {
    return sectionSize;
}

int Section::getPoolSize() {
    return pool.size();
}

void Section::mergeAddresses() {
    for(int i=0; i < addr.size(); i++) {
        mergedAddresses += addr[i];
    }
}

void Section::relocate(std::string file, std::string value, int offset) {
    std::vector<std::string> temp;
    for(int i = 0; i < 8; i+=2) 
        temp.push_back(value.substr(i, 2));
    
    std::string s = temp[3] + temp[1] + temp[1] + temp[0];
    
    // std::cout << s << std::endl;

    int fileIndex = 0;
    for(int j = 0; j < whichFile.size(); j++) {
        if(whichFile[j] == file) {
            fileIndex = j;
            break;
        }
    }

    // std::cout << addr[fileIndex] << std::endl;
    // std::cout << addr[fileIndex].substr(2*offset, 8) << std::endl;
    for(int k = 0; k < 8; k++) {
        addr[fileIndex].at(2*offset + k) = s[k];
    }
    // std::cout << addr[fileIndex].substr(2*offset, 8) << std::endl;
    // std::cout << addr[fileIndex] << std::endl;
}

void Section::relocateForSection(std::string file, unsigned int value, int offset) {
    int fileIndex = 0;
    for(int j = 0; j < whichFile.size(); j++) {
        if(whichFile[j] == file) {
            fileIndex = j;
            break;
        }
    }

    std::string addrToChange = addr[fileIndex].substr(2*offset, 8);
    unsigned int v = std::stoi(addrToChange);
    v += value;

    std::stringstream ss;
    ss << std::hex << v;
    std::string address = ss.str();
    std::vector<std::string> temp;
    for(int i = 0; i < 8; i+=2) 
        temp.push_back(address.substr(i, 2));
    
    std::string s = temp[3] + temp[1] + temp[1] + temp[0];

    for(int k = 0; k < 8; k++) {
        addr[fileIndex].at(2*offset + k) = s[k];
    }
}

unsigned int Section::getSectionSizeForLinker() {
    int size = 0;
    for(int i = 0; i < addr.size(); i++) 
        size += addr[i].length();
    return size/2;
}