#include <iostream>
#include <string>
#include <fstream>
using namespace std;
// Global vars
int nrT2;
int nrT1;
double correlation;
int nrGroups;
string overlapGroups;


void generateVarDividend(int begin, int end, int nrBins){
	int binSize = (end-begin)/nrBins;
	for(int dividendSize=begin; dividendSize<=end; dividendSize+=binSize){
		cout<<dividendSize<<endl;
	}
}

void initValues(){

}

int main(){
	initValues();
	generateVarDividend(10,100,10);

	return 0;
}
