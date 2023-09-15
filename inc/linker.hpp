#ifndef LINKER_H_
#define LINKER_H_

#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <map>
#include "../inc/section.hpp"
#include "../inc/symbol.hpp"

class Linker
{
private:
    std::map<std::string, unsigned int> sectionPlaces;

    std::vector<Section*> sections;
    std::vector<Symbol* > globalSymTable;
    // std::vector<std::string> allInstructions;
    // std::map<std::string, int> sectionNameAndSize;
    // std::map<std::string, int> secNameStartAddr;
    long lastAddr;
    bool isHex;
public:
    std::vector<std::string> order;
    std::string outputFileName;
    Linker();
    ~Linker();
    void setHex();
    void addToSectionPlaces(std::string sectionName, std::string address);
    void addToOrder(std::string fileName);
    int sectionExists(std::string secName);
    int symbolExistsLinker(std::string name);
    void updatedSectionStartAddr(std::vector<std::pair<std::string, unsigned int>> &vec, std::string section, std::string fromFile);
    unsigned int getStartAddr(std::vector<std::pair<std::string, unsigned int>> &vec, std::string section);
    void mapSections();
    void createSymTable();
    void fixRelocations();
    std::pair<int, bool> symbolIsSection(std::string sim);
    void mergeSections();
    void writeToHex();
    // std::string getSectionNameFromPlace(std::string value);
};

#endif