#include <string>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <utility>
#include "../inc/linker.hpp"

Linker::Linker(/* args */) {}

Linker::~Linker() {}

typedef std::pair<std::string, unsigned int> pair;

std::string getSectionNameFromPlace(std::string value) {
    int eq = value.find("=");
    int dis = value.find("@") - value.find("=");
    return value.substr(eq+1, dis-1);
}

std::string getAddressFromPlace(std::string value) {
    int et = value.find("@");
    return value.substr(et+1);
}

bool isOutpuFile(std::string value) {
    return value.find(".hex") != std::string::npos;
}

inline bool fileExists(const std::string& name) {
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

inline std::vector<pair> sortMap(std::map<std::string, unsigned int> mapa) {
    std::vector<pair> vec;

    std::copy(mapa.begin(),
            mapa.end(),
            std::back_inserter<std::vector<pair>>(vec));

    std::sort(vec.begin(), vec.end(), [](const pair &l, const pair &r) {
        if (l.second != r.second) {
            return l.second < r.second;
        }
 
        return l.first < r.first;
    });
    
    return vec;
}

 int main(int argc, char* argv[]) {
     std::string inputFile;
     std::string outputFile;
    
     Linker* linker = new Linker();

     if(argv[1] == "-hex") {
         linker->setHex();
     }

     if(argc > 0) {
         for(int i = 1; i < argc; i++) {
             std::string arg = (std::string)argv[i];

             if(arg.substr(0, 6) == "-place") {
                 linker->addToSectionPlaces(getSectionNameFromPlace(arg), getAddressFromPlace(arg));
             } else if(isOutpuFile(arg)) {
                 linker->outputFileName = arg.substr(0, arg.find(".hex"));
             } else if(arg.find(".o") != std::string::npos){
                 linker->addToOrder(arg.substr(0, arg.find(".o")));
             }
         }
    } else {
        std::cout << "ERROR: Missing arguments!" << std::endl;
     }

     for(int i = 0; i < linker->order.size(); i++) {
         std::string file= "../o/" + linker->order[i] + ".o";
        //  std::cout << file << std::endl;
         if(!fileExists(file)) {
            std::cout << "ERROR: Missing " + linker->order[i] + ".o"<< std::endl;
             exit(0);
         }
     }

     linker->mapSections();
     linker->createSymTable();
     linker->fixRelocations();
     linker->writeToHex();
     std::cout << "FINISHED LINKING!" << std::endl;;
     return 0;
 }


void Linker::mapSections() {
    // std::vector<std::map<std::string, int>> tempMap;

    for(int i = 0; i < order.size(); i++) {
        int numLines = 0;
        int numOfSections = 0;
        std::string line;
        std::ifstream myfile("../o/" + order[i] + ".o");

        while(numLines++ < 2) {
            std::getline(myfile, line);
        }

        numOfSections = std::stoi(line); 
        
        // std::cout << order[i] + " " + line << std::endl;
        // std::cout << "Iz fajla: " + order[i] << numOfSections << std::endl;
        for(int j = 0; j < numOfSections; j++) {
            std::getline(myfile, line);
            std::stringstream ss(line);
            int temp = 0;
            std::string secName;
            int secSize = 0;
            std::string instrs;
            while(temp++ < 4) {
                std::string t;
                ss >> t;
                // std::cout << t << std::endl;
                if(temp == 2) {
                    secName = t;
                } else if(temp == 3) {
                    secSize = std::stoi(t); 
                } else if(temp == 4) {
                    instrs = t;
                }
                // std::cout << secName + " " + std::to_string(secSize) << std::endl;
            }
            
            int index = sectionExists(secName); 
            if(index == -1) {
                Section* sec = new Section(secName);
                sec->sectionSize = secSize;
                sec->addr.push_back(instrs);
                sec->whichFile.push_back(order[i]);
                sections.push_back(sec);
                // std::cout << "Dodao sekciju:" + secName + " iz fajla " + order[i] + " " + sec->addr.back() + "   " + sec->whichFile.back() << std::endl;
            } else {
                sections[index]->sectionSize += secSize;
                sections[index]->whichFile.push_back(order[i]);
                sections[index]->addr.push_back(instrs);
                // std::cout << "2 Dodao za sekciju:" + secName + " iz fajla " + order[i] << std::endl;
            }
        }        
    }

    // for(int i = 0; i < sections.size(); i++) {
    //     std::cout << "Za " + sections[i]->getName() << std::endl;
    //     for(int j = 0; j<sections[i]->whichFile.size(); j++) {
    //         std::cout << sections[i]->whichFile[j] + "  ";
    //     }
    //     std::cout << std::endl; 
    // }

    unsigned int startAddr = 0;
    if(sectionPlaces.size() > 0) {
        std::vector<pair> places = sortMap(sectionPlaces);
        
        for (auto const &pair: places) {
            std::string sectionName = pair.first;
            
            startAddr = pair.second;
            int index = sectionExists(sectionName);
            if(index != -1) {
                sections[index]->startAddr = pair.second;
            } else {
                std::cout << "ERROR: Section doesn't exist!" << std::endl;
                exit(0);
            }

            startAddr += sections[index]->getSectionSizeForLinker();
            // std::cout << sectionName << std::endl;
            // std::cout << pair.second << std::endl;
        }

        for(int i = 0; i < sections.size(); i++) {
            if(sections[i]->startAddr == 0) {
                sections[i]->startAddr = startAddr;
                startAddr += sections[i]->getSectionSizeForLinker();
            }
        }
    } else {
        for(int i = 0; i < sections.size(); i++) {
            if(sections[i]->startAddr == 0) {
                sections[i]->startAddr = startAddr;
                startAddr += sections[i]->getSectionSizeForLinker();
            }
        }
    }

    std::sort(sections.begin(), sections.end(), [](const Section *lhs, const Section *rhs) {
        return lhs->startAddr < rhs->startAddr;
    });

    //check if sections can fit 
    for(int k = 0; k < sections.size()-1; k++) {
        if(sections[k]->startAddr + sections[k]->getSectionSizeForLinker() >  sections[k+1]->startAddr) {
            std::cout << "ERORR! Section " + sections[k]->getName() + " and section " + sections[k+1]->getName() + " overlap!" << std::endl;
            std::cout << sections[k]->getName() + " ending " << sections[k]->startAddr + sections[k]->getSectionSizeForLinker() << std::endl;
            std::cout << sections[k+1]->getName() + " starting " << sections[k+1]->startAddr << std::endl;
            exit(1);
        } else if(sections[k]->startAddr + sections[k]->getSectionSizeForLinker() > 4294967040) {
            std::cout << "ERORR! Section " + sections[k]->getName() + " overlaps with mapped registers!"  << std::endl;
            exit(1);
        }
    }

    for(int i = 0; i < sections.size(); i++) {
        if(symbolExistsLinker(sections[i]->getName()) == -1) {
            Symbol* sym = new Symbol(sections[i]->startAddr, sections[i]->getName(), sections[i]->getName());
            sym->setTypeToSection();
            globalSymTable.push_back(sym);
        } else {
            std::cout << "ERROR! Symbol with this name already exists!" << std::endl;
        }
        // std::cout << globalSymTable.back()->getName() << "\t \t " << globalSymTable.back()->valueForLinker  
        // << "\t \t " << globalSymTable.back()->getNum() << std::endl;
    }
        std::cout << std::endl;
}

void Linker::createSymTable() {
    std::vector<pair> sekcijaAddrStart;
    std::set<std::string> allSymbols;
    pair p;

    for(int i = 0; i < globalSymTable.size(); i++){
        p.first = globalSymTable[i]->getName();
        p.second = globalSymTable[i]->valueForLinker;        
        // allSymbols.insert(globalSymTable[i]->getName());
        sekcijaAddrStart.push_back(p);
    }
    std::vector<std::vector<Symbol*>> symTables;
    for(int i = 0; i < order.size(); i++) { //kroz svaki fajl
        int numLines = 0;
        int numOfSections = 0;
        std::string line;
        std::vector<Symbol*> tabela;
        std::ifstream myfile("../o/" + order[i] + ".o");
        while(numLines++ < 2) {
            std::getline(myfile, line);
        }
        numOfSections = std::stoi(line);
        for(int i = 0; i < numOfSections; i++) {
            std::getline(myfile, line);
        }
        std::getline(myfile, line);
        std::getline(myfile, line);
        int numOfSymbols = std::stoi(line);
        
        bool firstFlag = true;
        std::string prevSec = "";
        while(numOfSymbols-- > 0) {//kroz tabelu simbola u fajlu
            std::getline(myfile, line);
            std::stringstream ss(line);
            unsigned int startAddr = 0;
            std::string pom;
            // std::cout << numOfSymbols << std::endl;
            std::vector<std::string> params;
            for(int j = 0; j < 5; j++) {
                ss >> pom;
                params.push_back(pom);
                // std::cout << params.back() << std::endl;
            } 
            if(params[1] != "UND") {
                if(params[2] != "SCTN") {
                    int index = symbolExistsLinker(params[0]);
                    if(index == -1) {
                        int sectionIndex = symbolExistsLinker(params[1]);
                        if(sectionIndex != -1) {
                            if(firstFlag) {
                                firstFlag = false;
                                prevSec = params[1];
                            } else if ((prevSec != params[1]) ) {
                                // std::cout << prevSec <<" sad:" + params[1] << std::endl;
                                updatedSectionStartAddr(sekcijaAddrStart, prevSec, order[i]);
                            }
                            prevSec = params[1];
                            startAddr = getStartAddr(sekcijaAddrStart, params[1]) + std::stoi(params[3]);
                            Symbol* sym = new Symbol(startAddr, params[1], params[0]);
                            if(params[4] == "GLOB") 
                                sym->setGlobal();
                            globalSymTable.push_back(sym);
                            tabela.push_back(sym);

                            // std::cout << tabela.back()->getName() + " " << tabela.back()->sectionForLinker + " " << tabela.back()->valueForLinker << std::endl;
                        } else {
                            std::cout << "ERROR! Section " + params[1] + "doesn't exist!" << std::endl;
                            exit(0); 
                        }
                    } else {
                        std::cout << "ERROR! Symbol " + params[0] + " already exists in another file!" << std::endl;
                        exit(0);
                    }
                }
            }
            if(numOfSymbols == 0) {
                // std::cout << numOfSymbols << " " + params[1] <<std::endl;
                updatedSectionStartAddr(sekcijaAddrStart, params[1], order[i]);
            }
        }
        // symTables.push_back(tabela);
    }
    // for(int z = 0; z < globalSymTable.size(); z++){
    //     std::string location = globalSymTable[z]->getType() == 1 ? "SCTN" : "NOTYP";
    //     std::cout << globalSymTable[z]->getName() + "\t\t" << globalSymTable[z]->valueForLinker << "\t\t" + location + "\t\t" <<  globalSymTable[z]->sectionForLinker + "\t\t" << std::endl;
    // }
}

void Linker::fixRelocations() {
    for(int i = 0; i < order.size(); i++) { //kroz svaki fajl
        int numLines = 0;
        int numOfSections = 0;
        std::string line;
        std::vector<Symbol*> tabela;
        std::ifstream myfile("../o/" + order[i] + ".o");
        // std::cout << "U FAJLU: " + order[i] << std::endl;
        while(numLines++ < 2) {
            std::getline(myfile, line);
        }
        numOfSections = std::stoi(line); 
        for(int i = 0; i < numOfSections; i++) {
            std::getline(myfile, line);
        }
        std::getline(myfile, line);
        std::getline(myfile, line);
        int numOfSymbols = std::stoi(line);\
        
        
        while(numOfSymbols-- > 0) {//kroz tabelu simbola u fajlu
            std::getline(myfile, line);
        }

        std::getline(myfile, line);
        std::getline(myfile, line);
        int numOfRelocationTables = std::stoi(line);
        for(int j = 0; j<numOfRelocationTables; j++) {
            std::getline(myfile, line);
            std::string sectionName = line;
            std::getline(myfile, line);
            int numOfRelocations = std::stoi(line);

            // std::cout << "Relok za sek: " + sectionName << std::endl;
            for(int k = 0; k<numOfRelocations; k++) {
                std::getline(myfile, line);
                std::stringstream ss(line);
                std::vector<std::string> params;
                for(int z = 0; z < 4; z++) {
                    std::string pom;
                    ss >> pom;
                    params.push_back(pom);
                }
                
                std::pair<int, bool> par = symbolIsSection(params[3]);
                //jeste sekcija
                if(par.second) {
                    unsigned int addrSimbola = globalSymTable[par.first]->valueForLinker;
                    std::stringstream stream;
                    int offset = std::stoi(params[1]);
                    int sectionIndex = sectionExists(sectionName);
                    if(sectionIndex != -1) {
                        sections[sectionIndex]->relocateForSection(order[i], addrSimbola , offset);
                    }
                } else { // nije sekcija 
                    unsigned int addrSimbola = globalSymTable[par.first]->valueForLinker;
                    std::stringstream stream;
                    stream << std::hex << addrSimbola;
                    // std::cout << "VALUE: " +  stream.str() << std::endl;
                    int offset = std::stoi(params[1]);
                    int sectionIndex = sectionExists(sectionName);
                    if(sectionIndex != -1) {
                        sections[sectionIndex]->relocate(order[i], stream.str() , offset);
                    }
                }
            }
            //razmak nakon svake relok tabele
            std::getline(myfile, line);
        }
    }
}

void Linker::mergeSections() {
    std::vector<Section*> temp;
    int k = 0;
    for(int i = 0; i <sections.size(); i++){
        // std::cout << "USAO" + sections[k]->name + " " << sections[k]->startAddr << "  "<< sections[k]->getSectionSizeForLinker() << std::endl;
        if(i == 0) 
            temp.push_back(sections[i]);
        else {
            if(sections[k]->startAddr + sections[k]->getSectionSizeForLinker() == sections[i]->startAddr) {
                sections[k]->addr.push_back(sections[i]->mergedAddresses);
                // std::cout << "Dodao adrese iz sekcije" + sections[i]->name + "u sekciju " + sections[k]->name << std::endl;
            } else {
                if(k!=0) {
                    temp.push_back(sections[k]);
                    // std::cout << "ubacio sekciju " + sections[k]->name<< std::endl;
                }
                k = i;
            }
        }
        if(i == sections.size()-1)
            temp.push_back(sections[k]);
    }
    
    for(int i =0; i < temp.size(); i++) {
       temp[i]->mergeAddresses(); 
    }
    sections = temp;
}

void Linker::writeToHex() {
    std::ofstream outfile ("../hex/" + outputFileName + ".hex");

    outfile << "#" + outputFileName + ".hex" << std::endl;
    for(int i =0; i < sections.size(); i++) {
       sections[i]->mergeAddresses();
    }

    mergeSections();

    std::stringstream ss;
    for(int i = 0; i < sections.size(); i++) {
        std::string wholeSection = "";
        for(int j = 0; j < sections[i]->addr.size(); j++) {
            wholeSection += sections[i]->addr.at(j);    
        }

        int count = 0;
        for(int k = 0; k < wholeSection.length()/2; k++) {
            if(k % 8 == 0) {
                ss << std::hex << sections[i]->startAddr + count*8;
                outfile << std::endl;
                std::string adresa = ss.str();
                transform(adresa.begin(), adresa.end(), adresa.begin(), ::toupper);
                outfile << adresa << ": ";
                ss.str(std::string());
                count++;
            }
            std::string bajt = wholeSection.substr(k*2, 2);
            transform(bajt.begin(), bajt.end(), bajt.begin(), ::toupper);
            outfile << bajt + " "; 
        }
        if(i < sections.size() - 1){
            if(sections[i]->startAddr + sections[i]->getSectionSizeForLinker() == sections[i+1]->startAddr) {
                //nista
            } else {
                outfile << std::endl;
            }
        } 
        
    }

    outfile.close();
}

std::pair<int, bool> Linker::symbolIsSection(std::string sim) {
    int j = 0;
    for(int i = 0; i < globalSymTable.size(); i++) {
        if(globalSymTable[i]->getName() == sim) {
            if(globalSymTable[i]->getType() == 1) 
                return std::make_pair(i, true);
            else 
                return std::make_pair(i, false);
        }
        j = i;
    }
    return std::make_pair(j, false);;
}

int Linker::symbolExistsLinker(std::string name) {
    for(int i = 0; i < globalSymTable.size(); i++){
        if(globalSymTable[i]->getName() == name) 
            return i;
    }
    return -1;
}

int Linker::sectionExists(std::string secName) {
    for(int i = 0; i < sections.size(); i++) {
        if(sections[i]->getName() == secName) {
            return i;
        }
    }	
    return -1;
}

unsigned int Linker::getStartAddr(std::vector<std::pair<std::string, unsigned int>> &vec, std::string section) {
    for(int i = 0; i < vec.size(); i++) {
        if(vec[i].first == section)
            return vec[i].second;
    }
    return 0;
}

void Linker::updatedSectionStartAddr(std::vector<std::pair<std::string, unsigned int>> &vec, std::string section, std::string fromFile) {
    int sectionIndex = 0;
    for(int i = 0; i < sections.size(); i++) {
        if(sections[i]->getName() == section) {
            sectionIndex = i;
            break;
        }
    }

    std::string instrs;
    for(int i = 0; i < sections[sectionIndex]->whichFile.size(); i++) {
        if(sections[sectionIndex]->whichFile[i] == fromFile) {
            instrs = sections[sectionIndex]->addr[i];
            break;
        } 
    }

    int v = instrs.length() / 2;
    int index = 0;
    for(int i = 0; i < vec.size(); i++) {
        if(vec[i].first == section) {
            index = i;
            break;
        } 
    }
    vec[index].second += v;
}

void Linker::setHex() {
    isHex = true;
}

void Linker::addToSectionPlaces(std::string sectionName, std::string address) {
    unsigned int x = std::stoul(address, nullptr, 16);
    sectionPlaces[sectionName] = x;
}

void Linker::addToOrder(std::string fileName) {
    order.push_back(fileName);
}
