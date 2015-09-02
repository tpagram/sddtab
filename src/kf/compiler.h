#ifndef COMPILER_H
#define COMPILER_H

extern "C" {
   #include "sddapi.h" 
}
#include "KFormula.h"
#include <iostream>
#include <map>
namespace compiler {
    SddNode* KtoSDD(const KFormula*, SddManager*);
    int setLiterals(std::vector<KFormula*>, std::vector<KFormula*>& literalsToAtoms, std::map<const KFormula*, int, KFormula::less_than_unpack>& atomsToLiterals);
}
#endif
