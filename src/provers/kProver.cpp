#include "provers/kProver.h"

KProver::KProver(std::vector<KFormula*> literalsToAtoms, 
                    std::map<const KFormula*, int, KFormula::less_than_unpack> atomsToLiterals) {
    this->literalsToAtoms = literalsToAtoms;
    this->atomsToLiterals = atomsToLiterals;
    literalsToChildren.resize(literalsToAtoms.size() + 1);
}

bool KProver::isSatisfiable(SddNode* alpha, SddManager* m) {
    std::unordered_set<SddLiteral> responsibleVars;
    return isSatisfiable(alpha,m,responsibleVars);
}

/*
Returns whether an SDD is satisfiable under modal logic k.
*/
bool KProver::isSatisfiable(SddNode* alpha, SddManager* m, std::unordered_set<SddLiteral>& responsibleVars) {
            
    //Match against satCache
    if (satCache.count(alpha) == 1) return true;
    
    //Base cases
    if (sdd_node_is_true(alpha)) return true;
    if (sdd_node_is_false(alpha)) return false;
        
    //Scrape off a single satisfying set, corresponding to a single open tableaux branch.
    std::vector<SddLiteral> branch = getOpenBranch(alpha, std::vector<SddLiteral>());
    
    //Extract all modal atoms from the branch.
    std::vector<SddLiteral> boxes;
    std::vector<SddLiteral> negBoxes;
    if (extractModals(branch,boxes,negBoxes) == 0) return true;
    
    //If there are no negative boxes, our branch is open.
    if (negBoxes.empty()) return true;
        
    //First half of modal jump: unboxing all boxed variables
    SddNode* unboxed = sdd_manager_true(m);
    SddNode* tmp;
    for (SddLiteral i : boxes) {
        unboxed = sdd_conjoin(tmp = unboxed, compiler::KtoSDD(&literalsToAtoms[i]->getleft(),m),m);
        sdd_ref(unboxed,m); sdd_deref(tmp,m);
        if (sdd_node_is_false(unboxed)) {
            SddNode* beta = refineSdd(findMinimalSubset(m,i,boxes,responsibleVars), alpha, m);
            sdd_ref(beta,m);
            
            std::unordered_set<SddLiteral> postRefinedResVars;
            bool isSat = isSatisfiable(beta,m,postRefinedResVars);
            if (isSat) {
                if (cacheSat(beta)) {
                    sdd_deref(beta,m);
                }
            }   
            else {
                sdd_deref(beta,m);
                responsibleVars.insert(postRefinedResVars.begin(), postRefinedResVars.end());
            }
            
            sdd_deref(unboxed,m);
            
            return isSat;
        }
    }
        
    //Second half of modal jump: unbox a negative boxed variable
    SddNode* nextWorld;
    for (SddLiteral i : negBoxes) {
        nextWorld = sdd_conjoin(unboxed, sdd_negate(compiler::KtoSDD(&literalsToAtoms[-i]->getleft(),m),m),m);
        sdd_ref(nextWorld,m);
        if (sdd_node_is_false(nextWorld)) {
            SddNode* beta = refineSdd(findMinimalSubset(m,i,boxes,responsibleVars), alpha, m);
            sdd_ref(beta,m);
            
            std::unordered_set<SddLiteral> postRefinedResVars;
            bool isSat = isSatisfiable(beta,m,postRefinedResVars);
            if (isSat) {
                if (cacheSat(beta)) {
                    sdd_deref(beta,m);
                }
            }   
            else {
                sdd_deref(beta,m);
                responsibleVars.insert(postRefinedResVars.begin(), postRefinedResVars.end());
            }
            
            sdd_deref(unboxed,m); sdd_deref(nextWorld,m); 
            
            return isSat;
        }
                
        std::unordered_set<SddLiteral> postJumpResVars;
        bool isSat = isSatisfiable(nextWorld,m,postJumpResVars);
        
        if (!isSat) {
            
            boxes.push_back(i); SddNode* beta = refineSdd(boxes, alpha, m);
            //SddNode* beta = refineSdd(findMinimalSubsetPostJump(m,i,boxes,responsibleVars,postJumpResVars), alpha, m);
            sdd_ref(beta,m);
            
            std::unordered_set<SddLiteral> postRefinedResVars;
            bool isSat = isSatisfiable(beta,m,postRefinedResVars);
            if (isSat) {
                if (cacheSat(beta)) {
                    sdd_deref(beta,m);
                }
            }   
            else {
                sdd_deref(beta,m);
                responsibleVars.insert(postRefinedResVars.begin(), postRefinedResVars.end());
            }
            sdd_deref(unboxed,m); sdd_deref(nextWorld,m);
            
            return isSat;
        }
    }

    sdd_deref(unboxed,m);
    
    cacheSat(alpha);

    return true;
}

/*
Given a vector of literals and an SDD, adds the negation of those literals to the SDD.
*/
SddNode* KProver::refineSdd(std::vector<SddLiteral> literals, SddNode* oldSdd, SddManager* m) {
        SddNode* tmp;
        SddNode* newSdd = sdd_manager_false(m);
        for (SddLiteral lit : literals) {
            newSdd = sdd_disjoin(tmp = newSdd, sdd_manager_literal(-lit,m),m);
            sdd_ref(newSdd,m); sdd_deref(tmp,m);
        }
        newSdd = sdd_conjoin(oldSdd,tmp = newSdd,m);
        sdd_deref(tmp,m);
        return newSdd;
}

/*
    Finds a minimal subset of literals that, together with a known false-creating literal, create a false SDD.
*/
std::vector<SddLiteral> KProver::findMinimalSubset(SddManager* m, SddLiteral known, std::vector<SddLiteral> possible,
                                                        std::unordered_set<SddLiteral>& responsibleVars) {
    SddNode* minimalSdd;
    if (known < 0) {
        minimalSdd = sdd_conjoin(sdd_manager_true(m),
                                            sdd_negate(compiler::KtoSDD(&literalsToAtoms[-known]->getleft(),m),m),m);
    }
    else {
        minimalSdd = sdd_conjoin(sdd_manager_true(m),compiler::KtoSDD(&literalsToAtoms[known]->getleft(),m),m);
    }
    sdd_ref(minimalSdd,m);
    std::vector<SddLiteral> minLits;
    minLits.push_back(known);
    responsibleVars.insert(known);
    SddNode* unboxedSdd;
    SddNode* tmp;
    int endIt = possible.size();
    while (!sdd_node_is_false(minimalSdd)) {
        
        unboxedSdd = sdd_conjoin(minimalSdd,sdd_manager_true(m),m);
        sdd_ref(unboxedSdd,m);
                
        for (int i = 0; i < endIt; i++) {
            unboxedSdd = sdd_conjoin(tmp = unboxedSdd, compiler::KtoSDD(&literalsToAtoms[possible[i]]->getleft(),m),m);
            sdd_deref(tmp,m); sdd_ref(unboxedSdd,m);
                    
            if (sdd_node_is_false(unboxedSdd)) {
                minLits.push_back(possible[i]);
                responsibleVars.insert(possible[i]);
                        
                minimalSdd = sdd_conjoin(tmp = minimalSdd, compiler::KtoSDD(&literalsToAtoms[possible[i]]->getleft(),m),m);
                sdd_deref(tmp,m); sdd_deref(unboxedSdd,m); sdd_ref(minimalSdd,m);
                endIt = i;
                break;
            }
        }
    }
    
    sdd_deref(minimalSdd,m);
    
    return minLits;
}

std::vector<SddLiteral> KProver::findMinimalSubsetPostJump(SddManager* m, SddLiteral known, std::vector<SddLiteral> possible,
                                                        std::unordered_set<SddLiteral>& responsibleVars,
                                                        std::unordered_set<SddLiteral>& postJumpResVars) {
    std::vector<SddLiteral> minLits;      
    bool newPostResVarsAdded = true;
    while (newPostResVarsAdded) {
	    newPostResVarsAdded = false;
	    for (SddLiteral i : possible) {
		    if (responsibleVars.count(i) != 0) {
		    } 
		    else if (shareAnElement(postJumpResVars, getChildren(i))) {
                minLits.push_back(i);
			    responsibleVars.insert(i);
			    postJumpResVars.insert(getChildren(i).begin(),
										getChildren(i).end());
			    newPostResVarsAdded = true;
	        }
	    }
        if (responsibleVars.count(known) != 0) {
		} 
		else if (shareAnElement(postJumpResVars, getChildren(known))) {
            minLits.push_back(known);
		    responsibleVars.insert(known);

		    postJumpResVars.insert(getChildren(known).begin(),
									getChildren(known).end());
		    newPostResVarsAdded = true;
		}
    }
    return minLits;
}

//Returns true if alpha should be derefed.
bool KProver::cacheSat(SddNode* alpha) {
    if (satCache.count(alpha) == 0)	{
	    if (satCache.size() < maxCacheSize) {
		    satCacheDeque.push_back(alpha);
		    satCache.insert(alpha);
            return false;
		}
        return true;
	}
    return false;
}

std::unordered_set<SddLiteral>& KProver::getChildren(SddLiteral i) {
    i = abs(i);
	if (literalsToChildren.at(i).empty()) {
		KFormula::computeChildren(&literalsToAtoms[i]->getleft(), literalsToChildren.at(i));
	}
	return literalsToChildren.at(i);
}