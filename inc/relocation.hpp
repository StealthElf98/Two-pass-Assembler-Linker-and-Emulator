#ifndef RELOCATION_H_
#define RELOCATION_H_

#include <string>
#include <vector>

class Relocation {
private:
    int offset;
    int type;
    int num;
public:
    Relocation(int offser, int type, int num);
    ~Relocation();
};

#endif