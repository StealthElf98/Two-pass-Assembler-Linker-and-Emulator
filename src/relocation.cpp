#include <string>
#include "../inc/relocation.hpp"

Relocation::Relocation(int o, int t, int a): offset(o), type(t), addend(a) {}

Relocation::~Relocation(){}