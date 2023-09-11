#ifndef LINKER_H_
#define LINKER_H_

#include <string>
#include <iostream>
#include <vector>
#include <sstream>

class Linker
{
private:
    std::vector<std::string> assemblyOrder;
public:
    Linker(/* args */);
    ~Linker();
};

#endif