#include "s4Prover.h"

S4Prover::S4Prover(std::vector<KFormula*> literalsToAtoms, 
                    std::map<const KFormula*, int, KFormula::less_than_unpack> atomsToLiterals) {
    this->literalsToAtoms = literalsToAtoms;
    this->atomsToLiterals = atomsToLiterals;
    literalsToChildren.resize(literalsToAtoms.size() + 1);
}

bool S4Prover::isSatisfiable(SddNode* alpha, SddManager* m) {
    std::unordered_set<SddNode*,SddHasher> assumedSatSdds;
    std::unordered_set<SddLiteral> responsibleVars;
    return isSatisfiable(alpha,m,std::unordered_set<SddLiteral>(),sdd_manager_true(m), assumedSatSdds, responsibleVars);
    }

bool S4Prover::isSatisfiable(SddNode* alpha, SddManager* m, std::unordered_set<SddLiteral> permVars, SddNode* permSdd,
                                std::unordered_set<SddNode*,SddHasher>& assumedSatSdds , std::unordered_set<SddLiteral>& responsibleVars) {
    
    //Match against cache
    if (satCache.count(alpha) == 1) {
        return true;
    }
    
    //Base cases
    if (sdd_node_is_true(alpha)) {
        return true;
    }
    
    if (sdd_node_is_false(alpha)) {
        return false;
    }
    
    //Scrape off a single satisfying set, corresponding to a single open tableaux branch.
    std::vector<SddLiteral> branch = getOpenBranch(alpha, std::vector<SddLiteral>());
    SddNode* branchSdd = branchToSDD(branch,m);
    sdd_ref(branchSdd,m);

    /*std::cout << "BRANCH\n\n";
    for (SddLiteral i : branch) {
        if (i < 0) std::cout << "~" << *literalsToAtoms[-i] << "\n\n";
        else std::cout << *literalsToAtoms[i] << "\n\n";
    }
    std::cout << "BRANCH END\n\n";*/
    
    //Extract all modal atoms from the branch.
    std::vector<SddLiteral> boxes;
    std::vector<SddLiteral> negBoxes;
    if (extractModals(branch,boxes,negBoxes) == 0) {
        return true;
    };

    //Checks for new boxed variables
    SddNode* tmp;
    std::unordered_set<SddLiteral> newPermVars = permVars;
    std::vector<SddLiteral> unboxed;
    SddNode* postUnboxingBranch = sdd_conjoin(sdd_manager_true(m),branchSdd,m);
    sdd_ref(postUnboxingBranch,m);
    SddNode* newPermSdd = sdd_conjoin(sdd_manager_true(m),permSdd,m);
    sdd_ref(newPermSdd,m);
    for (SddLiteral i : boxes) {
        if (newPermVars.count(i) == 0) {
            unboxed.push_back(i);
            newPermVars.insert(i);
            postUnboxingBranch = sdd_conjoin(tmp = postUnboxingBranch, compiler::KtoSDD(&literalsToAtoms[i]->getleft(),m),m);
            sdd_deref(tmp,m); sdd_ref(postUnboxingBranch,m);
            newPermSdd = sdd_conjoin(tmp = newPermSdd, compiler::KtoSDD(&literalsToAtoms[i]->getleft(),m),m);
            sdd_deref(tmp,m); sdd_ref(newPermSdd,m);
            newPermSdd = sdd_conjoin(tmp = newPermSdd, compiler::KtoSDD(literalsToAtoms[i],m),m);
            sdd_deref(tmp,m); sdd_ref(newPermSdd,m);
            //If becomes false while unboxing, recurse.
            if (sdd_node_is_false(postUnboxingBranch)) {

                //Determine minimal set of unboxed variables
                std::vector<SddLiteral> minLits;
                minLits.push_back(i);
                responsibleVars.insert(i);
                std::vector<SddLiteral>::iterator endIt = --(unboxed.end());
                SddNode* minimalSdd = sdd_conjoin(compiler::KtoSDD(&literalsToAtoms[i]->getleft(),m),branchSdd,m);
                sdd_ref(minimalSdd,m);
                SddNode* minBoxVarsSdd = sdd_conjoin(compiler::KtoSDD(&literalsToAtoms[i]->getleft(),m),sdd_manager_true(m),m);
                sdd_ref(minBoxVarsSdd,m);                       
                while (true) {
                    sdd_deref(postUnboxingBranch,m);
                    postUnboxingBranch = sdd_conjoin(minimalSdd, sdd_manager_true(m), m);
                    sdd_ref(postUnboxingBranch,m);
                    if (sdd_node_is_false(postUnboxingBranch)) break; //Last one added made it false
                    for (std::vector<SddLiteral>::iterator v = unboxed.begin();
							v != endIt; ++v) {
                        postUnboxingBranch = sdd_conjoin(tmp = postUnboxingBranch, compiler::KtoSDD(&literalsToAtoms[*v]->getleft(),m),m);
                        sdd_deref(tmp,m); sdd_ref(postUnboxingBranch,m);
						if (sdd_node_is_false(postUnboxingBranch)) {
							//Last one added made it false
							minimalSdd = sdd_conjoin(tmp = minimalSdd, compiler::KtoSDD(&literalsToAtoms[*v]->getleft(),m),m);
                            sdd_deref(tmp,m); sdd_ref(minimalSdd,m);
                            minBoxVarsSdd = sdd_conjoin(tmp = minBoxVarsSdd,compiler::KtoSDD(&literalsToAtoms[*v]->getleft(),m),m);
                            sdd_deref(tmp,m); sdd_ref(minBoxVarsSdd,m);
                            minLits.push_back(*v);
                            responsibleVars.insert(*v);
							endIt = v;
							break;
						}
					}
                }
                // Determine minimal set of branch variables together with unboxed variables
                std::vector<SddLiteral>::iterator BendIt = branch.end();
                sdd_deref(minimalSdd,m);
                minimalSdd = sdd_conjoin(sdd_manager_true(m),minBoxVarsSdd,m);
                sdd_ref(minimalSdd,m);
				while (true) {
                    sdd_deref(minBoxVarsSdd,m);
					minBoxVarsSdd = sdd_conjoin(minimalSdd, sdd_manager_true(m), m);
                    sdd_ref(minBoxVarsSdd,m);
                    if (sdd_node_is_false(minBoxVarsSdd)) break; //Last one added made it false
					for (std::vector<SddLiteral>::iterator v = branch.begin();
						v != BendIt; ++v) {
                        minBoxVarsSdd = sdd_conjoin(tmp = minBoxVarsSdd, sdd_manager_literal(*v,m), m);
                        sdd_deref(tmp,m); sdd_ref(minBoxVarsSdd,m);
                        if (sdd_node_is_false(minBoxVarsSdd)) {
    						//Last one added made it false
                            minimalSdd = sdd_conjoin(tmp = minimalSdd, sdd_manager_literal(*v,m),m);
                            sdd_deref(tmp,m); sdd_ref(minimalSdd,m);
							minLits.push_back(*v);
                            responsibleVars.insert(*v);
                            BendIt = v;
							break;
						}
					}
				}
                sdd_deref(minimalSdd,m); sdd_deref(postUnboxingBranch,m); sdd_deref(minBoxVarsSdd,m);
                
                //std::cout << *literalsToAtoms[minLits[0]] << " and " << *literalsToAtoms[minLits[1]] << "\n";
                SddNode* beta = refineSdd(minLits, alpha, m);
                sdd_ref(beta,m);
                dependentSdds.insert(alpha);
                bool isSat = isSatisfiableRefined(beta, m, permVars, permSdd, assumedSatSdds, responsibleVars);
                dependentSdds.erase(alpha);
                sdd_deref(beta,m); sdd_deref(branchSdd,m); sdd_deref(newPermSdd,m);
                return isSat;
            }
        }
    }
    // There were new boxes, so recurse.
    if (!unboxed.empty()) {
        dependentSdds.insert(alpha);
        std::unordered_set<SddLiteral> postResponsibleVars;
        if (!isSatisfiable(postUnboxingBranch,m,newPermVars,newPermSdd,assumedSatSdds, postResponsibleVars)) {
            std::vector<SddLiteral> minLits;
            for (SddLiteral v : branch) {
                if (postResponsibleVars.count(v) == 1) {
                    minLits.push_back(v);
                    responsibleVars.insert(v);
                }
            }
			for (SddLiteral v : unboxed) {
				std::unordered_set<SddLiteral> children;
				KFormula::computeChildrenBoxS4(&literalsToAtoms[v]->getleft(), children);
				if (shareAnElement(postResponsibleVars, children)) {
                    minLits.push_back(v);
                    responsibleVars.insert(v);
				}
			}
               SddNode* beta = refineSdd(minLits, alpha, m);
               sdd_ref(beta,m);
               bool isSat = isSatisfiableRefined(beta, m, permVars, permSdd, assumedSatSdds, responsibleVars);
               sdd_deref(beta,m); sdd_deref(branchSdd,m); sdd_deref(newPermSdd,m); sdd_deref(postUnboxingBranch,m);
               dependentSdds.erase(alpha);
               return isSat;
        }
        sdd_deref(branchSdd,m); sdd_deref(newPermSdd,m);
        dependentSdds.erase(alpha);
        return true;
    }
    
    // There are no new boxes, so start undiamonding.
    sdd_deref(branchSdd,m);
    if (negBoxes.empty()) {
		return true;
	}
	dependentSdds.insert(alpha);
	SddNode* nextWorld;
    for (SddLiteral i : negBoxes) {
        nextWorld = sdd_conjoin(permSdd, sdd_negate(compiler::KtoSDD(&literalsToAtoms[-i]->getleft(),m),m),m);
        sdd_ref(nextWorld,m);        
        
        if (sdd_node_is_false(nextWorld)) {
            
            sdd_deref(nextWorld,m);
            nextWorld = sdd_conjoin(sdd_manager_true(m), sdd_negate(compiler::KtoSDD(&literalsToAtoms[-i]->getleft(),m),m),m);
            sdd_ref(nextWorld,m);
			responsibleVars.insert(i);
			std::vector<SddLiteral> minLits;
            minLits.push_back(i);
			
			// Determine a minimal unsatisfiable subset.
			std::unordered_set<SddLiteral>::iterator endIt = permVars.end();
            SddNode* minimalSdd = sdd_conjoin(nextWorld, sdd_manager_true(m),m);
            sdd_ref(minimalSdd,m);
			while (true) {
                sdd_deref(nextWorld,m);
                nextWorld = sdd_conjoin(minimalSdd, sdd_manager_true(m), m);
                sdd_ref(nextWorld,m);
				if (sdd_node_is_false(nextWorld)) break;
				for (std::unordered_set<SddLiteral>::iterator v = permVars.begin();
							v != endIt; ++v) {
                    nextWorld = sdd_conjoin(tmp = nextWorld,compiler::KtoSDD(&literalsToAtoms[*v]->getleft(),m) ,m);
                    sdd_deref(tmp, m); sdd_ref(nextWorld,m);
                    nextWorld = sdd_conjoin(tmp = nextWorld,sdd_manager_literal(*v,m) ,m);
                    sdd_deref(tmp, m); sdd_ref(nextWorld,m);
					if (sdd_node_is_false(nextWorld)) {
                        minimalSdd = sdd_conjoin(tmp = minimalSdd,compiler::KtoSDD(&literalsToAtoms[*v]->getleft(),m) ,m);
                        sdd_deref(tmp, m); sdd_ref(minimalSdd,m);
                        minimalSdd = sdd_conjoin(tmp = minimalSdd,sdd_manager_literal(*v,m) ,m);
                        sdd_deref(tmp, m); sdd_ref(minimalSdd,m);
						minLits.push_back(*v);
						responsibleVars.insert(*v);
                        endIt = v;
						break;
					}
				}
			} 
			SddNode* beta = refineSdd(minLits, alpha, m);
            sdd_ref(beta,m);
            bool isSat = isSatisfiableRefined(beta, m, permVars, permSdd, assumedSatSdds, responsibleVars);
            sdd_deref(beta,m); sdd_deref(nextWorld,m); sdd_deref(minimalSdd,m);
            dependentSdds.erase(alpha);
            return isSat;	  
        }
        if (dependentSdds.count(nextWorld) == 1) {
            //Loop detected
            assumedSatSdds.insert(nextWorld);
            continue;
        }
        std::unordered_set<SddNode*,SddHasher> postModalAssumedSatSdds = assumedSatSdds;
        std::unordered_set<SddLiteral> postResponsibleVars;
        if (!isSatisfiable(nextWorld,m,permVars,permSdd,postModalAssumedSatSdds, postResponsibleVars)) {
            
            std::vector<SddLiteral> minLits;

			bool newPostModalJumpResVarsAdded = true;
			while (newPostModalJumpResVarsAdded) {
				newPostModalJumpResVarsAdded = false;
				for (std::unordered_set<SddLiteral>::iterator v = permVars.begin();
						v != permVars.end(); ++v) {
					if (responsibleVars.count(*v) != 0) {
						continue;
					}
					std::unordered_set<SddLiteral> children = getChildren(*v);
					children.insert(*v);
					if (shareAnElement(postResponsibleVars, children)) {
                        minLits.push_back(*v);
						responsibleVars.insert(*v);
						// Add in other children to postModalJumpResVars
						postResponsibleVars.insert(children.begin(),
													children.end());
						newPostModalJumpResVarsAdded = true;
					}
				}
				if (responsibleVars.count(i) != 0) {
					// Don't bother, we've already accounted for this dia var.
				} else if (shareAnElement(postResponsibleVars, getChildren(i))) {
					// Note, <>phi stored as []~phi, thus the nith.
					minLits.push_back(i);
					responsibleVars.insert(i);
					// Add in other children to postModalJumpResVars
					postResponsibleVars.insert(getChildren(i).begin(),
												getChildren(i).end());
					newPostModalJumpResVarsAdded = true;
				}
			}
	
            SddNode* beta = refineSdd(minLits, alpha, m);
            sdd_ref(beta,m);
            bool isSat = isSatisfiableRefined(beta, m, permVars, permSdd, assumedSatSdds, responsibleVars);
            sdd_deref(beta,m); sdd_deref(nextWorld,m);
            dependentSdds.erase(alpha);
            return isSat;
        }
        assumedSatSdds.insert(postModalAssumedSatSdds.begin(),postModalAssumedSatSdds.end());
    }
    
    satCache.insert(alpha);
    dependentSdds.erase(alpha);
    return true;
}

SddNode* S4Prover::refineSdd(std::vector<SddLiteral> literals, SddNode* oldSdd, SddManager* m) {
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

bool S4Prover::isSatisfiableRefined(SddNode* alpha, SddManager* m, std::unordered_set<SddLiteral> permVars, SddNode* permSdd,
                                std::unordered_set<SddNode*,SddHasher>& assumedSatSdds, std::unordered_set<SddLiteral>& responsibleVars) {
    assumedSatSdds.clear();
    std::unordered_set<SddLiteral> postResponsibleVars;
    bool isSat;
    if (sdd_node_is_false(alpha)) {
        isSat = false;
    }
    else if (dependentSdds.count(alpha) == 1) {
        //Loop detected
        assumedSatSdds.insert(alpha);
        isSat = true;
    }
    else {
        isSat = isSatisfiable(alpha,m,permVars,permSdd,assumedSatSdds, postResponsibleVars);
    }
    
    if (assumedSatSdds.count(alpha) == 1) {
		assumedSatSdds.erase(alpha);
	}
    
    if (!isSat) {
        responsibleVars.insert(postResponsibleVars.begin(), postResponsibleVars.end());
    }
    dependentSdds.erase(alpha);
    return isSat;
}

std::unordered_set<SddLiteral>& S4Prover::getChildren(SddLiteral i) {
    i = std::abs(i);
	if (literalsToChildren.at(i).empty()) {
		KFormula::computeChildren(&literalsToAtoms[i]->getleft(), literalsToChildren.at(i));
	}
	return literalsToChildren.at(i);
}
