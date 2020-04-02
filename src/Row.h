//
// Created by gonzaga on 9/14/15.
//

#ifndef DATAGENERATOR_ROW_H
#define DATAGENERATOR_ROW_H

#include<vector>
#include<string>
#include "Attribute.h"

using namespace std;

class Value
{
public:
    vector<float> fValues;
    string strValue;
};

class Row
{
public:
    vector<Value> values;
};


#endif //DATAGENERATOR_ROW_H
