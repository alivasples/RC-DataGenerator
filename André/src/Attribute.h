//
// Created by gonzaga on 8/31/15.
//

#ifndef DATAGENERATOR_ATTRIBUTE_H
#define DATAGENERATOR_ATTRIBUTE_H

#include<map>
#include<vector>

using namespace std;

enum DISTRIBUTION{NORMAL=0,UNIFORM=1,EXPONENCIAL=2};
enum DISTANCE{EUCLIDIAN=3,LEDIT=4};

class Attribute
{
    public:

    string name;
    string type;
    //TODO: METRIC TYPE (VARCHAR, VARFLOAT)
    int size;
    DISTANCE distanceFunction;
    float threshold;

    vector<DISTRIBUTION> distributions;
    //TODO: CHANGE TO P1, P2, ETC...
    vector<float> mean;
    vector<float> stdDev;


    Attribute(string name, string type, int size, DISTANCE distFunction, float threshold, vector<DISTRIBUTION> distributions, vector<float> mean, vector<float> stddev)
    {
        this->name = name;
        this->type = type;
        this->size = size;
        this->distanceFunction = distFunction;
        this->threshold = threshold;
        this->distributions = distributions;
        this->mean = mean;
        this->stdDev = stddev;
    };

    Attribute(string name, string type, int size)
    {
        this->name = name;
        this->type = type;
        this->size = size;
    };

    void toString();
};


#endif //DATAGENERATOR_ATTRIBUTE_H
