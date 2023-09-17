#ifndef EMULATOR_H_
#define EMULATOR_H_

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>

class Emulator
{
private:

public:
    Emulator();
    ~Emulator();
    std::map<std::string, std::string> code;
    void readFile(std::string fileName);
    void emulate();
    void movePC();
    std::string calculateAddress(std::string gprA, std::string gprB, std::string D);
    std::string getValueFromReg(std::string reg);
    std::string convertFromLittleEndian(std::string value);
    void setRegisterValue(std::string reg, std::string value);
    std::string getValueFromCSR(std::string num);
    void setValueForCSR(std::string num, std::string value);
    std::string makeFourBytes(std::string value);
    void printRegisterValues();
    std::string pc;
    std::string sp;
    std::vector<std::string> regs;
    std::vector<std::string> csr;
    std::vector<std::string> stack;
};

#endif