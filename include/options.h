#ifndef OPTIONS_H
#define OPTIONS_H

#include <iostream>

struct Options {
    bool globalAssumptions;
    std::string logic;
    bool verbose;
    bool satisfying;
    std::string outputFile;
    bool reportSize;
    
    Options(int argc, char **argv);
    void printHelp();
};

#endif