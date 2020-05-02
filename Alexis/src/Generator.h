#ifndef GENERATOR_H
#define GENERATOR_H

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <random>
#include <cmath>

#include <simple_file_parser/simple_file_parser.h>
#include <Attribute.h>
#include <Row.h>

#define DBG_MSG(msg) if(isDebugMode) cout<<msg;
#define DBG_INST(inst) if(isDebugMode) inst;

using namespace std;

map<string, int> dictionary = {
    {"NORMAL", 0},
    {"UNIFORM", 1},
    {"EXPONENCIAL", 2},
    {"EUCLIDIAN", 3},
    {"LEDIT", 4}
};

class Generator{
    private:
        // ATTRIBUTES
        bool isDebugMode;
        simple_file_parser sfp; // Input parser
        ofstream fContent; // Before called Dividend (File)
        ofstream fRequisites; // Before called Divisor  (File)
        ofstream fGroups;  // Groups where content tuples belong to (File)
        int nrT1; // number of tuples of content table 
        int nrT2; // number of tuples of requisites table
        int nrGroups; // number of groups to divide the content table (T1)
        double correlation; // correlation
        double stdMean;
        double dev;
        bool withOutliers; // flag for outliers
        double overlapping; // percentage of groups overlapping
        DISTRIBUTION startGroupsDist;
        DISTRIBUTION fromGroupsDist;
        DISTRIBUTION toGroupsDist;
        int nrT1Atts; // The number of attributes of T1
        int nrT2Atts; // The number of attributes of T2
        vector<Attribute> attsT1; // Collection of specifications for every attribute of T1
        vector<Attribute> attsT2; // Collection of specifications for every attribute of T2
        vector<set<int> > tGroups; //Table of Groups
        default_random_engine generator; // random generator
        vector<Row> tuplesT1; // Collection of tupples values for every att of T1
        vector<Row> tuplesT2; // Collection of tupples values for every att of T2
        
        // METHODS
        void parseReqsTableAttributes();
        void parseContentTableAttributes();
        int getRandomGroup(DISTRIBUTION distribution);
        float getRandomFloatNumber(DISTRIBUTION distribution, float p1, float p2);
        string getRandomStringValue(DISTRIBUTION distribution, int size);

    public:
        // METHODS
        // Constructor
        Generator(string inputFileName);
        // Destructor
        ~Generator();
        // Getters and Setters
        void setDebugMode(bool isDebugMode);
        // Other methods
        void parseParameters();
        void distributeGroups();
        void populateT1();
        void populateT2();
        void saveFiles();
};

// PRIVATE METHODS

// Reading T1 Attributes. Example:
// ATT1(float, 2) EUCLIDIAN 0.25
// UNIFORM 0 1
// UNIFORM 0 1
void Generator::parseContentTableAttributes(){
    // Vars definition
    string attName, attType;
    int attSize;
    DISTANCE attDistance;
    double attThreshold, attMean, attStdDev;
    DISTRIBUTION attDistribution;
    // Reading all attributes from input
    for (int i = 0; i < nrT1Atts; ++i)
    {
        sfp.get_next_line();
        //parse attribute properties
        attName = sfp.get_token(0); // ATT1
        attType = sfp.get_token(2); // float
        attSize = atoi(sfp.get_token(4).c_str()); // 2
        attDistance = DISTANCE(dictionary[sfp.get_token(6)]); // EUCLIDIAN
        attThreshold = atof(sfp.get_token(7).c_str()); // 0.25

        vector<DISTRIBUTION> distributions; // (UNIFORM, UNIFORM)
        vector<float> mean; // (0,0)
        vector<float> dev; // (1,1)

        if(attType == "float")
        {
            for(int j = 0; j < attSize; j++)
            {
                sfp.get_next_line();
                attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]); // UNIFORM
                attMean = atof(sfp.get_token(1).c_str()); // 0
                attStdDev = atof(sfp.get_token(2).c_str()); // 1
                // Push current row data to their vectors
                distributions.push_back(attDistribution);
                mean.push_back(attMean);
                dev.push_back(attStdDev);
            }
        }
        else // strings
        {
            sfp.get_next_line();
            attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]);
            distributions.push_back(attDistribution);
        }

        attsT1.push_back(Attribute(attName, attType, attSize, attDistance, attThreshold, distributions, mean, dev));
    }
}

// Reading T2 Attributes (too similar to T1 reader). EXAMPLE:
// ATT1(float, 2) EUCLIDIAN 0.25
// UNIFORM 0 1
// UNIFORM 0 1
void Generator::parseReqsTableAttributes(){
    // Vars definition
    string attName, attType;
    int attSize;
    DISTANCE attDistance;
    double attThreshold, attMean, attStdDev;
    DISTRIBUTION attDistribution;
    // Reading all attributes from input
    for (int i = 0; i < nrT2Atts; ++i)
    {
        sfp.get_next_line();
        //parse attribute properties
        attName = sfp.get_token(0); // ATT1
        attType = sfp.get_token(2); // float
        attSize = atoi(sfp.get_token(4).c_str()); // 2
        attDistance = DISTANCE(dictionary[sfp.get_token(6)]); // EUCLIDIAN
        attThreshold = atof(sfp.get_token(7).c_str()); // 0.25

        vector<DISTRIBUTION> distributions; // (UNIFORM, UNIFORM)
        vector<float> mean; // (0,0)
        vector<float> dev; // (1,1)

        if(attType == "float")
        {
            for(int j = 0; j < attSize; j++)
            {
                sfp.get_next_line();
                attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]); // UNIFORM
                attMean = atof(sfp.get_token(1).c_str()); // 0
                attStdDev = atof(sfp.get_token(2).c_str()); // 1
                // Push current row data to their vectors
                distributions.push_back(attDistribution);
                mean.push_back(attMean);
                dev.push_back(attStdDev);
            }
        }
        else // strings
        {
            sfp.get_next_line();
            attDistribution = DISTRIBUTION(dictionary[sfp.get_token(0)]);
            distributions.push_back(attDistribution);
        }

        attsT2.push_back(Attribute(attName, attType, attSize, attDistance, attThreshold, distributions, mean, dev));
    }
}

// This function returns an index of group (distributed)
int Generator::getRandomGroup(DISTRIBUTION distribution)
{
    switch(distribution)
    {
        case DISTRIBUTION::NORMAL:
        {
            normal_distribution<float> normalDistribution(stdMean,dev);

            return (int)normalDistribution(generator) % nrGroups;
        }
        case DISTRIBUTION::UNIFORM:
        {
            uniform_int_distribution<int> uniformDistribution(0, nrGroups-1);
            return uniformDistribution(generator);
        }
        case DISTRIBUTION::EXPONENCIAL:
        {
            int N = nrGroups / 2;
            float totalAreaUntilN = 0.8;

            float lambda = -log(1 - totalAreaUntilN) / N;

            exponential_distribution<float> expDistribution(lambda);
            return (int)expDistribution(generator) % nrGroups;
        }
    }

    cout << endl << "ERROR: distribution not identified!";
    return -1;
}

// This function will return a random float number
float Generator::getRandomFloatNumber(DISTRIBUTION distribution, float p1, float p2)
{
    switch(distribution)
    {
        case DISTRIBUTION::NORMAL:
        {
            // Here p1 is mean and p2 is stantard deviation
            normal_distribution<float> normalDistribution(p1, p2);
            return normalDistribution(generator);
        }
        case DISTRIBUTION::UNIFORM:
        {
            // Here p1 and p2 are the lower and upper limits
            uniform_real_distribution<float> uniformDistribution(p1, p2);
            return uniformDistribution(generator);
        }
        case DISTRIBUTION::EXPONENCIAL:
        {
            // Here p1 is lamnda
            exponential_distribution<float> expDistribution(p1);
            return expDistribution(generator);
        }
    }

    return -1;
}

string Generator::getRandomStringValue(DISTRIBUTION distribution, int size)
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

// PUBLIC METHODS
// Constructor
Generator::Generator(string inputFileName){
    //sfp.open(inputFileName.c_str());
    fContent.open("T1.data");
    fRequisites.open("T2.data");
    fGroups.open("TG.data");
    // 3 decimal precision for float values in T1 and T2
    fContent << setprecision(3);
    fRequisites << setprecision(3);
    // For random values
    srand(time(NULL));
}

// Destructor
Generator::~Generator(){
    fContent.close();
    fRequisites.close();
    fGroups.close();
}

// Getters and Setters
void Generator::setDebugMode(bool isDebugMode){ this->isDebugMode = isDebugMode; }

// OTHER PUBLIC METHODS
// Read and parse input parameters
void Generator::parseParameters(){
    // Tokens for separate input lines
    sfp.set_single_char_tokens(", ()");
    // Getting the number of tuples for table of requisites (T2)
    sfp.get_next_line();
    nrT2 = sfp.get_token_int(0);
    // Getting the number of tuples for table of content (T1)
    sfp.get_next_line();
    nrT1 = sfp.get_token_int(0);
    // Getting the correlation
    sfp.get_next_line();
    correlation = atof(sfp.get_token(0).c_str());
    // Getting the number of groups
    sfp.get_next_line();
    nrGroups = sfp.get_token_int(0);
    // Calculating the mean and standard deviation
    stdMean = (float)nrGroups/2.0;
    dev = sqrt((float)nrGroups/4.0);

    // Getting the tag with/out outliers
    sfp.get_next_line();
    withOutliers = (sfp.get_token(1).compare("true") == false);
    // Groups overlapping factor of groups
    sfp.get_next_line();
    overlapping = atof(sfp.get_token(0).c_str());
    // 
    sfp.get_next_line();
    startGroupsDist = (DISTRIBUTION)dictionary[sfp.get_token(0)];
    //
    sfp.get_next_line();
    fromGroupsDist = (DISTRIBUTION)dictionary[sfp.get_token(0)];
    //
    sfp.get_next_line();
    toGroupsDist = (DISTRIBUTION)dictionary[sfp.get_token(0)];
    // Getting the number of requisites table (T2) attributes
    sfp.get_next_line();
    nrT1Atts = sfp.get_token_int(0);
    parseReqsTableAttributes();
    // Getting the number of content table (T1) attributes
    sfp.get_next_line();
    nrT2Atts = sfp.get_token_int(0);
    parseContentTableAttributes();
}

// Read and parse input parameters
void Generator::distributeGroups(){
    // Fill all the groups with the first <nrGroups> Content Table tuples
    for(int i = 0; i < nrGroups; i++){
        set<int> tmp;
        tmp.insert(i);
        tGroups.push_back(tmp);
    }

    // Distribute the rest of T1 tuples in the groups
    for(int i = nrGroups; i < nrT1; i++){
        tGroups[getRandomGroup(startGroupsDist)].insert(i);
    }

    //sharing tuples between groups...
    set<int>::iterator it;
    int fromGrp;
    int toGrp;
    int randomTupleFromGrp;

    for(int i = 0; i < overlapping * nrT1 ; i++)
    {
        do
        {
            // Pick a group to copy a tuple id
            fromGrp = getRandomGroup(fromGroupsDist);
            // Pick a group to paste a tuple id
            toGrp = getRandomGroup(toGroupsDist);
            // Initializing the iterator
            it = tGroups[fromGrp].begin();
            // Let's ddvance the iterator in random. positions
            randomTupleFromGrp = rand() % tGroups[fromGrp].size();
            advance(it,randomTupleFromGrp);

        // Validation, recompute if destiny group already has that tuple
        } while(tGroups[toGrp].find(*it) != tGroups[toGrp].end());
        // Insert the picked value 
        tGroups[toGrp].insert(*it);
    }
}

// Populate Content Table (T1)
void Generator::populateT1(){
    for(int i=0 ; i < nrT1; i++)
    {
        // Empty row of elements
        Row newRow;
        // Push a value for every attribute in T1
        for(Attribute& attribute: attsT1)
        {
            Value newValue;
            // Float value
            if(attribute.getType() == "float")
            {
                for (int j = 0; j < attribute.getSize(); j++)
                {
                    newValue.fValues.push_back(getRandomFloatNumber(attribute.getDistributionAt(j), attribute.getMeanAt(j), attribute.getStdDevAt(j)));
                }
            }
            // String value
            else newValue.strValue.assign(getRandomStringValue(attribute.getDistributionAt(0),attribute.getSize()));
            // Push the current value to the current tuple
            newRow.values.push_back(newValue);
        }
        // Push the new tuple to the T1 tupples set
        tuplesT1.push_back(newRow);
    }
}

// Populate Requisites Table (T2)
void Generator::populateT2(){

    for(int i=0 ; i < nrT2; i++)
    {
        //tSatisfyRequirements.push_back(set<int>());
        // Empty row of elements 
        Row newRow;
        // Push a value for every attribute in T2
        for(Attribute& attribute: attsT2)
        {
            Value newValue;
            // Float value
            if(attribute.getType() == "float")
            {
                for (int j = 0; j < attribute.getSize(); j++)
                {
                    newValue.fValues.push_back(getRandomFloatNumber(attribute.getDistributionAt(j), attribute.getMeanAt(j), attribute.getStdDevAt(j)));
                }
            }
            // String value
            else newValue.strValue.assign(getRandomStringValue(attribute.getDistributionAt(0),attribute.getSize()));
            // Push the current value to the current tuple
            newRow.values.push_back(newValue);
        }
        // Push the new tuple to T2 tupples set
        tuplesT2.push_back(newRow);
    }
}

// Save data into the files
void Generator::saveFiles(){
    //OUTPUT GROUPS, e.g.
    // row_id, group
    set<int>::iterator it;
    for(int idxTuple = 0; idxTuple < tuplesT1.size(); idxTuple++)
    {
        // Print the tuple id
        fGroups << idxTuple << '\t';
        int groupCounter = 0;

        // Finding all the groups that current tuple belongs to
        for(int idxGroup = 0; idxGroup < tGroups.size(); idxGroup++)
        {
            it = tGroups.at(idxGroup).find(idxTuple);
            if(it != tGroups.at(idxTuple).end())
            {
                if(groupCounter > 0) fGroups << ",";
                fGroups << idxGroup;
                groupCounter++;
            }
        }

        // it the row doesn't belong to any group at the end
        if(groupCounter == 0)
        {
            // consider it an outlier (create a group with a single row)
            if(withOutliers)
            {
                set<int> outlierGroupSet;
                outlierGroupSet.insert(idxTuple);
                tGroups.push_back(outlierGroupSet);
                fGroups << tGroups.size();
            }
            // or, fit the row on a valid group (this operation doesn't affect the correlation factor)
            else
            {
                int someValidGroup = tValidGroups.at(rand() % tValidGroups.size());
                tGroups.at(someValidGroup).insert(idxTuple);
                fGroups << someValidGroup;
            }
        }

        fGroups << endl;
    }

    // OUTPUT T1 
    for(Attribute& att : attsT1)
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

    // OUTPUT T2 
    for(Attribute& att : attsT2)
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

#endif