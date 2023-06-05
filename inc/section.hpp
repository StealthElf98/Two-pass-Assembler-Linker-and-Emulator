#ifndef SECTION_H_
#define SECTION_H_

#include <string>
#include <vector>

class Section {
private:
    std::string name;
    std::vector<int> *addr;  
public:
    Section(std::string name);
    ~Section();
};

#endif