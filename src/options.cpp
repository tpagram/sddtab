#include <cstring>
#include "options.h"

Options::Options(int argc, char **argv) {
    globalAssumptions = false;
    logic = "K";
    verbose = false;
    satisfying = false;
    reportSize = false;
    
    if (argc > 5) printHelp();
	
	for (int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "-s4", 3) == 0) {
	            	logic = "S4";
    		} 	
		else if (strncmp(argv[i], "-k", 3) == 0) {
            		logic = "K";
		} 
		else if (strncmp(argv[i], "-v", 2) == 0) {
			verbose = true;
		}
		else if (strncmp(argv[i], "-sat", 4) == 0) {
			satisfying = true;
    		} 
		else if (strncmp(argv[i], "-size", 4) == 0) {
    			reportSize = true;			
		} 
		else if (strncmp(argv[i], "-o", 2) == 0) {
            		outputFile = argv[i+1]; i++;
		} 
		else if (strncmp(argv[i], "-help", 2) == 0 || 
			strncmp(argv[i], "--help", 2) == 0 ||
			strncmp(argv[i], "-h", 2) == 0) {
            		printHelp();
		} 
		else {
            		printHelp();
		}
	}
}
void Options::printHelp() {
	std::cout << 	"SDDtab is an SDD-based modal theorem prover for logics K and S4.\n\n" <<
			"SDDtab reads modal formulae from standard input with the following options:\n\n" <<
			"-k	Enabled by default. Solves the benchmark in modal logic K. Mutually exclusive with option -s4.\n" <<
			"-s4	Solves the benchmark in modal logic S4. Mutually exclusive with option -k.\n" <<
			"-v 	Enable verbose mode.\n" <<
			"-sat 	If enabled, SDDtab will decide satisfiability. If not enabled, SDDtab defaults to deciding validity.\n" <<
			"-size	SDDtab returns the size of the initial SDD constructed and does not solve the formulae.\n" <<
			"-help	Print this help message\n\n" <<
			"Example command:\n" <<
			"cat benchmarks/k/k_branch_n/3.txt.k | bin/sddtab -v\n";
    	exit(1);
}
