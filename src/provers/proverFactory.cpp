#include "provers/proverFactory.h"

Prover* proverFactory::getProver(std::string logic, std::vector<KFormula*> literalsToAtoms, 
                                    std::map<const KFormula*, int, KFormula::less_than_unpack> atomsToLiterals) {
    Prover* proverInstance = NULL;
    if (logic == "K") {
        proverInstance = new KProver(literalsToAtoms, atomsToLiterals);
    }
    else if (logic == "S4") {
        proverInstance = new S4Prover(literalsToAtoms, atomsToLiterals);
    }
    else {
	std::cerr << "Unknown logic!";
        assert(false);
    }
    return proverInstance;
}
