#ifndef PROVER_FACTORY_H
#define PROVER_FACTORY_H

#include "prover.h"
#include "s4Prover.h"
#include "kProver.h"

class proverFactory {
public:
    static Prover* getProver(std::string logic, std::vector<KFormula*>,
                                std::map<const KFormula*, int, KFormula::less_than_unpack>);
};

#endif