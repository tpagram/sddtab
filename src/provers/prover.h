#ifndef PROVER_H
#define PROVER_H

extern "C" {
   #include "sddapi.h" 
}
#include <iostream>
#include <vector>
#include "../kf/KFormula.h"
#include <cmath>
#include <map>
#include <unordered_set>
#include "../kf/compiler.h"

class Prover {
    
protected:
    std::string logic;
    std::vector<KFormula*> literalsToAtoms;
    //std::vector<std::unordered_set<SddLiteral>> literalsToChildren;
    std::map<const KFormula*, int, KFormula::less_than_unpack> atomsToLiterals;
    std::vector<SddLiteral> getOpenBranch(SddNode*,std::vector<SddLiteral>);
    SddNode* branchToSDD(std::vector<SddLiteral>, SddManager*);
    int extractModals(std::vector<SddLiteral>,std::vector<SddLiteral>& boxes,std::vector<SddLiteral>& negBoxes);
    bool shareAnElement(const std::unordered_set<SddLiteral>& firstSet, const std::unordered_set<SddLiteral>& secondSet);
    //std::unordered_set<SddLiteral>& getChildren(SddLiteral);
public:
    virtual bool isSatisfiable(SddNode*,SddManager*) = 0;
};

// Hasher for unordered_maps and unordered_sets of Sdds:
struct SddHasher {
		size_t operator()(SddNode* alpha) const {
            return sdd_id(alpha);
		}
};
#endif
