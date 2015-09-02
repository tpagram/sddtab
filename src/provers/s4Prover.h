#ifndef S4PROVER_H
#define S4PROVER_H
 
#include "prover.h"

class S4Prover : public Prover {
    
public:
    S4Prover(std::vector<KFormula*>, std::map<const KFormula*, int, KFormula::less_than_unpack>);
    bool isSatisfiable(SddNode*, SddManager*);
    
private:
    std::vector<std::unordered_set<SddLiteral>> literalsToChildren;
    bool isSatisfiable(SddNode*, SddManager*, std::unordered_set<SddLiteral>,SddNode*,
                        std::unordered_set<SddNode*,SddHasher>& assumedSatSdds, std::unordered_set<SddLiteral>& responsibleVars);
    bool isSatisfiableRefined(SddNode*, SddManager*, std::unordered_set<SddLiteral>, SddNode*,
                                std::unordered_set<SddNode*,SddHasher>& assumedSatSdds, std::unordered_set<SddLiteral>& responsibleVars);
    SddNode* refineSdd(std::vector<SddLiteral>, SddNode*, SddManager*);
    std::unordered_set<SddNode*,SddHasher> dependentSdds;
    std::unordered_set<SddNode*,SddHasher> satCache;
    std::unordered_set<SddLiteral>& getChildren(SddLiteral);
    
};


#endif