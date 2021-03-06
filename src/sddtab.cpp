extern "C" {
   #include "sddapi.h" 
}
#include <chrono>
#include "kf/compiler.h"
#include "options.h"
#include <iostream>
#include <fstream>
#include "kf/KFormula.h"
#include "provers/proverFactory.h"

std::vector<KFormula*> literalsToAtoms(1);
std::map<const KFormula*, int, KFormula::less_than_unpack> atomsToLiterals;

int main(int argc, char** argv) {
    
    //Read input and convert to KFormulas.
    Options* opts = new Options(argc,argv);
    KFormula* notPsiNNF;
    std::string input;
    getline (std::cin, input);
    if (opts->verbose) std::cout << "Building Kformulas from input...\n";
    if (input.length() == 0) {
        std::cout << "Empty formula is valid.";
        return 0;
    } else if (opts->satisfying) {
        notPsiNNF = KFormula::toBoxNNF(KFormula::parseKFormula(input.c_str()));
    } else {
        notPsiNNF = KFormula::toBoxNNF(new KFormula(KFormula::NOT,KFormula::parseKFormula(input.c_str()),NULL));
    }
    if (opts->verbose) std::cout << "	Kformulas built!\n";
   
    //Gather atoms from KFormulas and assign literals.
    if (opts->verbose) std::cout << "Assigning literals...\n";
    std::vector<KFormula*> atoms = KFormula::getAtoms(notPsiNNF);
    SddLiteral var_count = compiler::setLiterals(atoms, literalsToAtoms, atomsToLiterals);
    if (opts->verbose) {
        std::cout << "	Literals assigned!\n";
    }

    //Initialise SDD library.
    Vtree* vtree = sdd_vtree_new(var_count,"balanced");
    SddManager* m = sdd_manager_new(vtree);
    sdd_vtree_free(vtree);
    sdd_manager_auto_gc_and_minimize_on(m);
    //sdd_manager_set_minimize_function(sdd_vtree_minimize,m);

    //Build SDDs
    if (opts->verbose) std::cout << "Converting Kformulas to SDDs...\n";
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    SddNode* notPsiSdd = compiler::KtoSDD(notPsiNNF,m);
    sdd_ref(notPsiSdd,m);
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    auto SddDuration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    int SddCount = sdd_count(notPsiSdd);
    int SddSize = sdd_size(notPsiSdd);
    if (opts->reportSize) {
        std::cout << SddCount << "\n";
        return 0;
    }
    if (opts->verbose) std::cout << "	SDD conversion complete!\n";
    
    //Check satisfiability or validity
    Prover* prover = proverFactory::getProver(opts->logic,literalsToAtoms, atomsToLiterals);
    t1 = std::chrono::high_resolution_clock::now();
    bool isSat = prover->isSatisfiable(notPsiSdd,m);
    t2 = std::chrono::high_resolution_clock::now();
    auto SolveDuration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
    
    
    //Output results.
    if (opts->verbose) {
	std::cout << "Stats:\n";
        std::cout << "	Initial Lit count = " << var_count << "\n";
        std::cout << "	Initial SDD size = " << SddSize << "\n";
        std::cout << "	Initial SDD count = " << SddCount << "\n"; 
	std::cout << "	Final manager size = " << sdd_manager_size(m) << "\n";
        std::cout << "	Time to build = " << SddDuration << "\n";
        std::cout << "	Time to solve = " << SolveDuration << "\n";
        std::cout << "	Total time = " << SddDuration + SolveDuration << "\n\n";
    }
    if (opts->satisfying) {
        if (!isSat) std::cout << "Psi is not satisfiable!\n";
        else std::cout << "Psi is satisfiable!\n";
    }
    else {
        if (!isSat) std::cout << "Psi is valid!\n";
        else std::cout << "Psi is not valid!\n";
    }
    
    //Finish up.
    sdd_manager_free(m);
    return 0;
}

