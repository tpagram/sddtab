#include "kf/compiler.h"

//Converts a KFormula to a Sentential Decision Diagram.
SddNode* compiler::KtoSDD(const KFormula* formula, SddManager* m) {
	switch (formula->getop()) {
		case KFormula::AP:
		    //Unsure if necessary.
		    //return sdd_manager_literal(formula->getvar(),m);
		case KFormula::BOX:
			return sdd_manager_literal(formula->getvar(),m);
		case KFormula::NOT:
			// Missing K,S4 related differentiation.
			return sdd_manager_literal(- formula->getleft().getvar(),m);
		case KFormula::AND: {
		    //std::cout<< "conjoining\n";
            SddNode* left = KtoSDD(&(formula->getleft()),m);
            sdd_ref(left,m);
            SddNode* right = KtoSDD(&(formula->getright()),m);
            sdd_ref(right,m);
            SddNode* alpha = sdd_conjoin(left,right,m);
            sdd_deref(left,m); sdd_deref(right,m);
            //sdd_ref(alpha,m);
            //sdd_manager_minimize(m);
            //sdd_deref(alpha,m);
            return alpha;    
            //return sdd_conjoin(KtoSDD(&(formula->getleft()),m),KtoSDD(&(formula->getright()),m),m);
		}
		case KFormula::OR: {
		    //std::cout<< "disjoining\n";
            SddNode* left = KtoSDD(&(formula->getleft()),m);
            sdd_ref(left,m);
            SddNode* right = KtoSDD(&(formula->getright()),m);
            sdd_ref(right,m);
            SddNode* alpha = sdd_disjoin(left,right,m);
            sdd_deref(left,m);sdd_deref(right,m);
            //sdd_ref(alpha,m);
            //sdd_manager_minimize(m);
            //sdd_deref(alpha,m);
            return alpha;
            //return sdd_disjoin(KtoSDD(&(formula->getleft()),m),KtoSDD(&(formula->getright()),m),m);
		}
		case KFormula::TRUE:
			return sdd_manager_true(m);
		case KFormula::FALSE:
			return sdd_manager_false(m);
		default:
			assert(false);
	}
    exit(1);
}

// Given a vector of atoms, associates a literal (int value) to each distinct atom.
int compiler::setLiterals(std::vector<KFormula*> atoms, std::vector<KFormula*>& literalsToAtoms, std::map<const KFormula*, int, KFormula::less_than_unpack>& atomsToLiterals) {
    int numLits = 1;
	for (std::vector<KFormula*>::iterator it = atoms.begin(); it < atoms.end(); ++it) {
		if (atomsToLiterals.count(*it) == 0) {
			// Write the variable number onto the formula directly.
			(**it).setvar(numLits);
            //std::cout << **it << "\n";
			atomsToLiterals.insert(std::pair<const KFormula*, int>(*it, numLits));
            literalsToAtoms.push_back(*it);
            //std::cout << "atom = " << *(**it).toString() << "  var = " << numLits << std::endl; 
            ++numLits;
		} else {
			// Still write the variable number onto the formula directly.
			(**it).setvar(atomsToLiterals.at(*it));
			//std::cout << "atom = " << *(**it).toString() << "  var = " << atomsToLiterals.at(*it) << std::endl; 
		}
	}
    return numLits;
}

