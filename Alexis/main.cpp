#include <iostream>
#include "src/Generator.h"
using namespace std;


int main(int argc, char* argv[]){
    // Just a simple execution validation 
    if(argc == 1) { cout<<"Please enter the name of the input file for data generation"<<endl; return 0;}
    // Create a generator for the argument file
    Generator generator(argv[1]);
    generator.setDebugMode(true);
    generator.parseParameters();
    generator.distributeGroups();
    generator.populateT1();
    generator.populateT2();
    generator.saveFiles();
    generator.closeFiles();
    return 0;
}