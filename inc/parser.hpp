#ifndef PARSER_H_
#define PARSER_H_
#include <cstring>
#include <iostream>
#include <vector>

class Parser {
private:
    std::vector<std::string> allLines;
    int numOfLines;
public:
    Parser(std::string inputFile);
    void parse(std::string inputFile);
    std::vector<std::string> getAllLines();
    int getNumOfLines();
    ~Parser();
};

#endif