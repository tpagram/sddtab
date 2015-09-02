#include "options.h"

Options::Options(int argc, char **argv) {
    globalAssumptions = false;
    logic = "K";
    verbose = false;
    satisfying = false;
    
    if (argc > 5) printHelp();
	
	for (int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "-g", 2) == 0) {
			globalAssumptions = true;
		} else if (strncmp(argv[i], "-s4", 3) == 0) {
            logic = "S4";
    	} else if (strncmp(argv[i], "-k", 3) == 0) {
            logic = "K";
		} else if (strncmp(argv[i], "-v", 2) == 0) {
			verbose = true;
		} else if (strncmp(argv[i], "-sat", 4) == 0) {
			satisfying = true;
		} else if (strncmp(argv[i], "-o", 2) == 0) {
            outputFile = argv[i+1]; i++;
		} else {
            printHelp();
		}
	}
}
void Options::printHelp() {
    exit(1);
}