#include "provers/prover.h"

std::vector<SddLiteral> Prover::getOpenBranch(SddNode* alpha, std::vector<SddLiteral> variables) {
    if (sdd_node_is_literal(alpha)) {
        variables.push_back(sdd_node_literal(alpha));
        return variables;
    }
    else {
        std::vector<SddLiteral> left = getOpenBranch(sdd_node_elements(alpha)[0],std::vector<SddLiteral>() );
        std::vector<SddLiteral> right = getOpenBranch(sdd_node_elements(alpha)[1],std::vector<SddLiteral>() );
        left.insert(left.end(), right.begin(), right.end());
        variables.insert(variables.end(), left.begin(), left.end());
        return variables;
    }
}

SddNode* Prover::branchToSDD(std::vector<SddLiteral> variables, SddManager* m) {
    SddNode* alpha = sdd_manager_true(m);
    SddNode* tmp;
    for (SddLiteral i: variables) {
            alpha = sdd_conjoin(tmp = alpha, sdd_manager_literal(i,m),m);
            sdd_ref(alpha,m); sdd_deref(tmp,m);
    }
    sdd_deref(alpha,m);
    return alpha;
}

int Prover::extractModals(std::vector<SddLiteral> variables, std::vector<SddLiteral>& boxes, std::vector<SddLiteral>& negBoxes) {
    int extracted = 0;
    for (SddLiteral i: variables) {
        if (literalsToAtoms[abs(i)]->getop() == KFormula::BOX) {
            extracted++;
            if (i >= 0) {
                boxes.push_back(i);
            }
            else if (i < 0) {
                negBoxes.push_back(i);
            }
        }
    }
    return extracted;
}

/*
 *	Determine whether two sets share any elements.
 */
bool Prover::shareAnElement(const std::unordered_set<SddLiteral>& firstSet,
					const std::unordered_set<SddLiteral>& secondSet) {
	if (firstSet.size() < secondSet.size()) {
		for (std::unordered_set<SddLiteral>::const_iterator it = firstSet.begin(); it != firstSet.end(); ++it) {
			if (secondSet.count(*it) != 0) {
				return true;
			}
		}
		return false;
	} else {
		for (std::unordered_set<SddLiteral>::const_iterator it = secondSet.begin(); it != secondSet.end(); ++it) {
			if (firstSet.count(*it) != 0) {
				return true;
			}
		}
		return false;
	}
}

/*std::unordered_set<SddLiteral>& Prover::getChildren(SddLiteral i) {
    i = abs(i);
	if (literalsToChildren.at(i).empty()) {
		KFormula::computeChildren(&literalsToAtoms[i]->getleft(), literalsToChildren.at(i));
	}
	return literalsToChildren.at(i);
}*/

