#ifndef PROBLEM_H
#define PROBLEM_H

extern "C" {
   #include <sddapi.h> 
}
#include <iostream>
#include <vector>

class Problem {
    
private:
    KFormula* parseTree;
    SddNode* compiledDiagram;
    bool satisfiable;
    
    
public:
};

#endif