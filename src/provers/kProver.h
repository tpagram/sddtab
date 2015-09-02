#ifndef KPROVER_H
#define KPROVER_H
 
#include "prover.h"

class KProver : public Prover {
public:
    KProver(std::vector<KFormula*>, std::map<const KFormula*, int, KFormula::less_than_unpack>);
    bool isSatisfiable(SddNode*,SddManager*);
private:
    bool isSatisfiable(SddNode*,SddManager*,std::unordered_set<SddLiteral>& responsibleVars);
    size_t maxCacheSize = 8000;
    std::unordered_set<SddNode*,SddHasher> satCache;
    std::deque<SddNode*> satCacheDeque;
    std::vector<SddLiteral> findMinimalSubset(SddManager*, SddLiteral, std::vector<SddLiteral>,
                                                std::unordered_set<SddLiteral>& responsibleVars);
    std::vector<SddLiteral> findMinimalSubsetPostJump(SddManager*, SddLiteral, std::vector<SddLiteral>,
                                                                std::unordered_set<SddLiteral>& responsibleVars,
                                                                std::unordered_set<SddLiteral>& postJumpResVars);
    SddNode* refineSdd(std::vector<SddLiteral>, SddNode*, SddManager*);
    bool cacheSat(SddNode*);
    std::unordered_set<SddLiteral>& getChildren(SddLiteral);
    std::vector<std::unordered_set<SddLiteral>> literalsToChildren;
};
 
#endif