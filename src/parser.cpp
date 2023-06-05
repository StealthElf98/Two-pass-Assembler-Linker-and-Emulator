#include "../inc/parser.hpp"
#include <fstream>
#include <string>
#include <vector>

Parser::Parser(std::string inputFile) {
    Parser::numOfLines = 0;
    std::string line;
    std::ifstream myfile(inputFile + ".txt");

    while (std::getline(myfile, line))
        ++numOfLines;
    
    parse(inputFile);
}

Parser::~Parser() {

}

std::vector<std::string> Parser::getAllLines() {
    return allLines;
}

int Parser::getNumOfLines() {
    return numOfLines;
}

void Parser::parse(std::string inputFile) {
    std::ifstream file(inputFile + ".txt");
    std::string str; 
    int i = 0;

    while (std::getline(file, str))
        allLines.push_back(str.substr(str.find_first_not_of(" \t")));
}