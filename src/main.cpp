#include <string>
#include <iostream>
#include <vector>
#include "../inc/parser.hpp"
#include "../inc/assembler.hpp"


int main(int argc, char* argv[]) {
    std::string inputFile;
    std::string outputFile;

    if(argc == 4) {
        if(std::strcmp(argv[1], "-o") != 0) {
            std::cout << "Missing -o" << std::endl;
            return 0;   
        }
        inputFile = argv[3];
        outputFile = argv[2];
     } else if (argc == 2) { 
        inputFile = argv[1];
        outputFile = "output.o";
    } else {
        std::cout << "Wrong number of arguments" << std::endl;    
    }

    Parser* parser = new Parser(inputFile);
    Assembler* assembler = new Assembler(outputFile);
    
    std::vector<std::string> parsedLines = parser->getAllLines();
    assembler->assemble(parsedLines);

    return 0;
}

