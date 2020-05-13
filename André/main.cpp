//TODO: group by algorithm based on attribute distance evaluator

#include <iostream>
#include <iterator>
#include <map>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <random>
#include <set>
#include <iomanip>
#include "src/Attribute.h"
#include "src/Row.h"
#include "include/simple_file_parser/simple_file_parser.h"
#include "src/Utils.h"
#include <queue>


#define SATISFACTION_MULTIPLIER 1

using namespace std;

map<string, int> dictionary = {{"NORMAL", 0},
                               {"UNIFORM", 1},
                               {"EXPONENCIAL", 2},
                               {"EUCLIDIAN", 3},
                               {"LEDIT", 4}};



//PARAMETERS -----------------------------------------------
int nRequeriments;
int nCandidates;
int nGroups;
float correlation;
float overlapping;
bool withOutliers;

DISTRIBUTION  startGroupsDist;
DISTRIBUTION  fromGroupsDist;
DISTRIBUTION  toGroupsDist;

int nAttRequeriments;
int nAttCandidates;


//AUX ------------------------------------------------------
simple_file_parser sfp;

// output files
fstream fDividend, fDivisor, fGroups;

//random values generator
std::random_device rd;
std::mt19937 generator(rd());

// attributes
vector<Attribute> attRequirements;
vector<Attribute> attCandidates;
string attName;
string attType;
int attSize;
DISTRIBUTION attDistribution;
DISTANCE attDistance;
float attMean;
float attStdDev;
float attThreshold;


//storage ---------------------------------------------------
//[{G1: 1,15,24}, {G2: 2,11}, ..., {G3: 3,44,41,18}]
vector<set<int>> tGroups;
vector<set<int>> tSatisfyRequirements;
vector<int> tValidGroups;

priority_queue<GroupPriority, vector<GroupPriority>, GroupPriority::PriorityCompare> groupsPriority;

vector<Row> t1;
vector<Row> t2;

float stdMean;
float dev;

int GetRandomGroup(DISTRIBUTION distribution)
{
    switch(distribution)
    {
        case DISTRIBUTION::NORMAL:
        {
            normal_distribution<float> normalDistribution(stdMean,dev);

            return (int)normalDistribution(generator) % nGroups;
        }
        case DISTRIBUTION::UNIFORM:
        {
            uniform_int_distribution<int> uniformDistribution(0, nGroups-1);
            return uniformDistribution(generator);
        }
        case DISTRIBUTION::EXPONENCIAL:
        {
            int N = nGroups / 2;
            float totalAreaUntilN = 0.8;

            float lambda = -log(1 - totalAreaUntilN) / N;

            exponential_distribution<float> expDistribution(lambda);
            return (int)expDistribution(generator) % nGroups;
        }
    }

    cout << endl << "ERROR: distribution not identified!";
    return -1;
}

float GenerateFloatValue(DISTRIBUTION distribution, float p1, float p2)
{
    switch(distribution)
    {
        case DISTRIBUTION::NORMAL:
        {
            normal_distribution<float> normalDistribution(p1, p2);
            return normalDistribution(generator);
        }
        case DISTRIBUTION::UNIFORM:
        {
            uniform_real_distribution<float> uniformDistribution(p1, p2);
            return uniformDistribution(generator);
        }
        case DISTRIBUTION::EXPONENCIAL:
        {
            exponential_distribution<float> expDistribution(p1);
            return expDistribution(generator);
        }
    }

    return -1;
}

string GenerateStringValues(DISTRIBUTION distribution, int size)
{
    int generatedNumber;

    string s = "";

    switch(distribution)
    {
        case DISTRIBUTION::NORMAL:
        {
            float std = (float)size/(float)2;
            float dev = sqrt((float)size/(float)4);

            normal_distribution<float> normalDistribution(std,dev);
            generatedNumber = (int)(normalDistribution(generator)) % size;
            break;
        }
        case DISTRIBUTION::UNIFORM:
        {
            uniform_int_distribution<int> uniformDistribution(0, size-1);
            generatedNumber = (int)uniformDistribution(generator);
            break;
        }
        case DISTRIBUTION::EXPONENCIAL:
        {
            int N = size / 2;
            float totalAreaUntilN = 0.8;

            float lambda = -log(1 - totalAreaUntilN) / N;

            exponential_distribution<float> expDistribution(lambda);
            generatedNumber = (int)expDistribution(generator) % size;
            break;
        }
    }

    for(int i=0; i <= generatedNumber; i++)
    {
        s.append("a");
    }

    return s;
}


void DistributeGroups()
{
    //groups have at least one tuple
    for(int i = 0; i < nGroups; i++)
    {
        set<int> tmp;
        tmp.insert(i);
        tGroups.push_back(tmp);
    }

    //distribute the rest...
    for(int i = nGroups; i < nCandidates; i++)
    {
        tGroups[GetRandomGroup(startGroupsDist)].insert(i);
    }


    //sharing tuples between groups...
    set<int>::iterator it;
    int fromGrp;
    int toGrp;
    int randomTupleFromGrp;

    for(int i = 0; i < overlapping * nCandidates ; i++)
    {
        do
        {
            fromGrp = GetRandomGroup(fromGroupsDist);
            toGrp = GetRandomGroup(toGroupsDist);

            it = tGroups[fromGrp].begin();

            advance(it,randomTupleFromGrp);

            randomTupleFromGrp = rand() % tGroups[fromGrp].size();

        }while(tGroups[toGrp].find(*it) != tGroups[toGrp].end());

        tGroups[toGrp].insert(*it);
    }
}

void GenerateCandidateValues()
{
    for(int i=0 ; i < nCandidates; i++)
    {
        Row newRow;

        for(Attribute& att: attCandidates)
        {
            Value newValue;

            if(att.type.compare("float") == 0)
            {
                for (int j = 0; j < att.size; j++)
                {

                    newValue.fValues.push_back(GenerateFloatValue(att.distributions.at(j), att.mean.at(j), att.stdDev.at(j)));
                }
            }
            else
            {
                newValue.strValue.assign(GenerateStringValues(att.distributions.at(0),att.size));
            }

            newRow.values.push_back(newValue);
        }

        t1.push_back(newRow);
    }

}

void GenerateRequirementsValues()
{

    for(int i=0 ; i < nRequeriments; i++)
    {
        tSatisfyRequirements.push_back(set<int>());
        Row newRow;

        for(Attribute& att: attRequirements)
        {
            Value newValue;
            if(att.type.compare("float") == 0)
            {
                for (int j = 0; j < att.size; j++)
                {

                    newValue.fValues.push_back(GenerateFloatValue(att.distributions.at(j), att.mean.at(j), att.stdDev.at(j)));

                }
            }
            else
            {
                newValue.strValue.assign(GenerateStringValues(att.distributions.at(0),att.size));
            }
            newRow.values.push_back(newValue);
        }

        t2.push_back(newRow);
    }

}


void ClearPQ()
{
    while(!groupsPriority.empty())
    {
        groupsPriority.pop();
    }
}

bool isGroupValid(int groupID)
{
    bool satisfied = false;
    int satisfiedRequirementsCounter = 0;
    set<int>::iterator it;

    GroupPriority groupPriority(groupID);

    for (int reqID = 0; reqID < t2.size(); ++reqID)
    {
        satisfied = false;

        Row requirement = t2.at(reqID);

        for (set<int>::iterator rowOfGroup = tGroups.at(groupID).begin(); rowOfGroup != tGroups.at(groupID).end(); rowOfGroup++)
        {
            Row tmpRow = t1.at(*rowOfGroup);

            int counter = 0;

            for (int j = 0; j < requirement.values.size(); ++j)
            {
                if(attRequirements.at(j).type.compare("float") == 0)
                {
                    if (Utils::Euclidian(tmpRow.values.at(j).fValues, requirement.values.at(j).fValues) <= attRequirements.at(j).threshold)
                    {
                        counter++;
                    }
                }
                else
                {
                    if (Utils::LEdit(tmpRow.values.at(j).strValue, requirement.values.at(j).strValue) <= attRequirements.at(j).threshold)
                    {
                        counter++;
                    }
                }
            }

            //if the row satisfies all the current requirement values
            if(counter == requirement.values.size())
            {
                //this requirement has been validate, stop here, go to next requirement
                satisfied = true;
                tSatisfyRequirements.at(reqID).insert(*rowOfGroup);
            }
        }

        if(!satisfied)
        {
            groupPriority.missedRequirements.push_back(reqID);
        }
        else
        {
            groupPriority.satisfiedRequirements.push_back(reqID);
            satisfiedRequirementsCounter++;
        }
    }

    groupsPriority.push(groupPriority);

    return satisfiedRequirementsCounter == t2.size();
}

void ParseParameters()
{
    sfp.set_single_char_tokens(", ()");

    sfp.get_next_line();
    nRequeriments = sfp.get_token_int(0);

    sfp.get_next_line();
    nCandidates = sfp.get_token_int(0);

    sfp.get_next_line();
    correlation = atof(sfp.get_token(0).c_str());

    sfp.get_next_line();
    nGroups = sfp.get_token_int(0);

    stdMean = (float)nGroups/2.0;
    dev = sqrt((float)nGroups/4.0);

    sfp.get_next_line();
    withOutliers = sfp.get_token(1).compare("true") == 0;

    sfp.get_next_line();
    overlapping = atof(sfp.get_token(0).c_str());

    sfp.get_next_line();
    startGroupsDist = (DISTRIBUTION)dictionary[sfp.get_token(0)];

    sfp.get_next_line();
    fromGroupsDist = (DISTRIBUTION)dictionary[sfp.get_token(0)];

    sfp.get_next_line();
    toGroupsDist = (DISTRIBUTION)dictionary[sfp.get_token(0)];

    //READ REQUIREMENTS -------------------------------------------------------------

    sfp.get_next_line();
    nAttRequeriments = sfp.get_token_int(0);

    for (int i = 0; i < nAttRequeriments; ++i)
    {
        sfp.get_next_line();
        //parse attribute properties
        attName = sfp.get_token(0);
        attType = sfp.get_token(2);
        attSize = atoi(sfp.get_token(4).c_str());
        attDistance = DISTANCE(dictionary[sfp.get_token(6)]);
        attThreshold = atof(sfp.get_token(7).c_str());

        vector<DISTRIBUTION> distributions;
        vector<float> mean;
        vector<float> dev;

        if(attType.compare("float") == 0)
        {
            for(int j = 0; j < attSize; j++)
            {
                sfp.get_next_line();
                attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]);
                attMean = atof(sfp.get_token(1).c_str());
                attStdDev = atof(sfp.get_token(2).c_str());

                distributions.push_back(attDistribution);
                mean.push_back(attMean);
                dev.push_back(attStdDev);
            }
        }
        else
        {
            sfp.get_next_line();
            attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]);
            distributions.push_back(attDistribution);
        }

        attRequirements.push_back(Attribute(attName,attType,attSize,attDistance,attThreshold,distributions,mean,dev));
    }

    //READ CANDIDATES --------------------------------------------------------------

    sfp.get_next_line();
    nAttCandidates = sfp.get_token_int(0);

    for (int i = 0; i < nAttCandidates; ++i)
    {
        sfp.get_next_line();
        //parse attribute properties
        attName = sfp.get_token(0);
        attType = sfp.get_token(2);
        attSize = atoi(sfp.get_token(4).c_str());
        attDistance = DISTANCE(dictionary[sfp.get_token(6)]);
        attThreshold = atof(sfp.get_token(7).c_str());

        vector<DISTRIBUTION> distributions;
        vector<float> mean;
        vector<float> dev;

        if(attType.compare("float") == 0)
        {
            for(int j = 0; j < attSize; j++)
            {
                sfp.get_next_line();
                attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]);
                attMean = atof(sfp.get_token(1).c_str());
                attStdDev = atof(sfp.get_token(2).c_str());

                distributions.push_back(attDistribution);
                mean.push_back(attMean);
                dev.push_back(attStdDev);
            }
        }
        else
        {
            sfp.get_next_line();
            attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]);
            distributions.push_back(attDistribution);
        }

        attCandidates.push_back(Attribute(attName, attType, attSize, attDistance, attThreshold, distributions, mean, dev));
    }


}

void FixCorrelationUp()
{
    GroupPriority goodGroup;

    do
    {
        goodGroup = groupsPriority.top();
        groupsPriority.pop();
    }while (goodGroup.missedRequirements.empty());


    for(auto requirementID : goodGroup.missedRequirements)
    {
        Row requirement = t2.at(requirementID);

        Row newRowToSatisfy;

        for (int i = 0; i < requirement.values.size(); ++i)
        {
            //TODO: do some variation on the values, instead of fully copy the missing requirement
            if(attRequirements.at(i).type.compare("float") == 0)
            {
                newRowToSatisfy.values.push_back(requirement.values.at(i));
            }
            else
            {
                newRowToSatisfy.values.push_back(requirement.values.at(i));
            }

        }


        for (int i = nAttRequeriments; i < nAttCandidates; ++i)
        {
            Attribute att = attCandidates.at(i);
            Value newValue;

            if(att.type.compare("float") == 0)
            {
                for (int j = 0; j < att.size; j++)
                {
                    newValue.fValues.push_back(GenerateFloatValue(att.distributions.at(j), att.mean.at(j), att.stdDev.at(j)));
                }
            }
            else
            {

                newValue.strValue = GenerateStringValues(att.distributions.at(0),att.size);
            }

            newRowToSatisfy.values.push_back(newValue);
        }

        //todo: check here...
        //for(int j=0 ; j < (rand() % SATISFACTION_MULTIPLIER)+1; j++)
        //{
            tGroups[goodGroup.id].insert(t1.size());
            t1.push_back(newRowToSatisfy);
        //}
    }

}

void FixCorrelationDown()
{
    GroupPriority goodGroup;

    set<int>::iterator it;

    goodGroup = groupsPriority.top();
    groupsPriority.pop();

    int requirementToRemove = goodGroup.satisfiedRequirements.back();

    goodGroup.satisfiedRequirements.pop_back();

    set<int> newGroupRowSet;

    set_difference(tGroups[goodGroup.id].begin(),tGroups[goodGroup.id].end(),
                   tSatisfyRequirements[requirementToRemove].begin(), tSatisfyRequirements[requirementToRemove].end(),
                   std::inserter(newGroupRowSet,newGroupRowSet.end()));

    tGroups[goodGroup.id] = newGroupRowSet;

}


int CountValidGroups()
{
    tValidGroups.clear();

    ClearPQ();

    int count = 0;

    for(int i=0; i < tGroups.size(); i++)
    {
        if(isGroupValid(i))
        {
            count++;
            tValidGroups.push_back(i);
        }
    }

    return count;
}

void OutputFiles()
{

    //OUTPUT GROUPS, e.g.
    // group_id row,row,row,row,row
    // group_id row,row,row
/*    for(int i = 0; i < tGroups.size(); i++)
    {
        fGroups << i << "\t";
        for (set<int>::iterator it = tGroups.at(i).begin(); it != tGroups.at(i).end(); it++)
        {
            fGroups << *it << ",";
        }
        fGroups << endl;
    }
*/

    //OUTPUT GROUPS, e.g.
    // row_id group, group, group
    // row_id group, group
    set<int>::iterator it;
    for(int i=0; i < t1.size(); i++)
    {
        fGroups << i << '\t';

        int groupCounter = 0;

        for(int g = 0; g < tGroups.size(); g++)
        {

            it = tGroups.at(g).find(i);

            if(it != tGroups.at(g).end())
            {
                if(groupCounter == 0)
                {
                    fGroups << g;
                }
                else
                {
                    fGroups << "," << g;
                }

                groupCounter++;
            }
        }

        //row doesn't have group at the end
        if(groupCounter == 0)
        {
            // consider it an outlier (create a group with a single row)
            if(withOutliers)
            {
                set<int> outlierGroupSet;
                outlierGroupSet.insert(i);
                tGroups.push_back(outlierGroupSet);
                fGroups << tGroups.size();
            }
            // or, fit the row on a valid group (this operation doesn't affect the correlation factor)
            else
            {
                int someValidGroup = tValidGroups.at(rand() % tValidGroups.size());
                tGroups.at(someValidGroup).insert(i);
                fGroups << someValidGroup;
            }
        }

        fGroups << endl;
    }

    //OUTPUT T1 ========================================================================================================
    for(Attribute& att : attCandidates)
    {
        fDividend << att.name << "(" << att.type << ","<< att.size <<");";
    }

    fDividend << endl;

    for(auto row: t1)
    {
        for(int i=0; i < row.values.size(); i++)
        {
            if(attCandidates.at(i).type.compare("float") == 0)
            {
                for(int j=0; j<row.values.at(i).fValues.size(); j++)
                {
                    if(j == row.values.at(i).fValues.size()-1)
                        fDividend << row.values.at(i).fValues.at(j);
                    else
                        fDividend << row.values.at(i).fValues.at(j) << ",";
                }
            }
            else
            {
                fDividend << row.values.at(i).strValue;
            }


            fDividend << ";";
        }
        fDividend << endl;
    }

    //OUTPUT T2 ========================================================================================================
    for(Attribute& att : attRequirements)
    {
        fDivisor << att.name << "(" << att.type << ","<< att.size <<");";
    }

    fDivisor << endl;

    for(auto row: t2)
    {
        for(int i=0; i < row.values.size(); i++)
        {
            if(attRequirements.at(i).type.compare("float") == 0)
            {
                for(int j=0; j<row.values.at(i).fValues.size(); j++)
                {
                    if(j == row.values.at(i).fValues.size()-1)
                        fDivisor << row.values.at(i).fValues.at(j);
                    else
                        fDivisor << row.values.at(i).fValues.at(j) << ",";
                }
            }
            else
            {
                fDivisor << row.values.at(i).strValue;
            }
            fDivisor << ";";
        }
        fDivisor << endl;
    }


}

void Init(string input)
{
    sfp.open(input);

    fDividend.open("T1.data", ios::out | ios::trunc);
    fDivisor.open("T2.data", ios::out | ios::trunc);
    fGroups.open("TG.data", ios::out | ios::trunc);

    fDivisor << setprecision(3);
    fDividend << setprecision(3);
}

void Terminate()
{
    fDividend.close();
    fDivisor.close();
    fGroups.close();
}


int main(int argc, char *argv[])
{

    cout << "Init" << "...";
    Init(argv[1]);


    cout << "ParseParameters" << "...";
    ParseParameters();


    cout << "DistributeGroups" << "...";
    DistributeGroups();


    cout << "GenerateRequirementsValues" << "...";
    GenerateRequirementsValues();


    cout << "GenerateCandidateValues" << "...";
    GenerateCandidateValues();


    int validGroupsCounter = CountValidGroups();

    int fixCount = 0;

    cout << endl << validGroupsCounter << "/" << tGroups.size() << endl;




    if(validGroupsCounter > correlation*nGroups)
    {
        fixCount++;
        for(int i = 0; i < validGroupsCounter - (correlation*nGroups) ; i++ )
        {
            cout << endl << "fixing down...";
            FixCorrelationDown();
        }
    }


    validGroupsCounter = CountValidGroups();


    if(validGroupsCounter < correlation*nGroups)
    {
        fixCount++;
        for(int i = 0; i < (correlation*nGroups) - validGroupsCounter ; i++ )
        {
            cout << endl << "fixing up...";
            FixCorrelationUp();
        }
    }

    if(fixCount == 2)
    {
        cout << "ERROR: FIXING GROUPS WENT WRONG !";
    }

    cout << endl << "counting again...";

    validGroupsCounter = CountValidGroups();

    cout << endl << validGroupsCounter << "/" << tGroups.size() << endl;

    cout << endl;
    for(auto g : tValidGroups)
    {
        cout << g << ", ";
    }

    cout << endl;

    OutputFiles();

    Terminate();

    return 0;
}