#include <string>
#include "../inc/relocation.hpp"

Relocation::Relocation(int o, int t, int n): offset(o), type(t), num(n) {}

Relocation::~Relocation(){}