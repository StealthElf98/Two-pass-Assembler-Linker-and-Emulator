#include "../inc/parser.hpp"
#include <fstream>
#include <string>
#include <vector>

Parser::Parser(std::string inputFile) {
    Parser::numOfLines = 0;
    std::string line;
    std::ifstream myfile("../tests/" + inputFile);

    std::cout<<inputFile << std::endl;
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
    std::ifstream file("../tests/" + inputFile);
    std::string str; 

    while(std::getline(file, str)) {
        if(!str.empty())
            allLines.push_back(str.substr(str.find_first_not_of(" ")));
    }
}