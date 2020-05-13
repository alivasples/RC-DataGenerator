//
// Created by gonzaga on 9/18/15.
//

#ifndef DATAGENERATOR_UTILS_H
#define DATAGENERATOR_UTILS_H

#include<vector>
#include<math.h>
#include <string>
#include<set>

using namespace std;

class GroupPriority
{
public:
    int id;
    vector<int> missedRequirements;
    vector<int> satisfiedRequirements;


    GroupPriority(int id)
    {
        this->id = id;
    };

    GroupPriority()
    {
    };

    struct PriorityCompare
    {
        bool operator()(const GroupPriority &g1, const GroupPriority &g2) const
        {
            return g1.missedRequirements.size() > g2.missedRequirements.size();
        }
    };
};


class Utils
{
public:
    static double Euclidian(vector<float> o1, vector<float> o2)
    {
        double distance = 0;

        for(int i=0; i < o1.size(); i++)
        {
            distance += pow((o1.at(i) - o2.at(i)),2.0);
        }

        return sqrt(distance);
    };

    static int LEdit(const std::string& s1, const std::string& s2)
    {
        const std::size_t len1 = s1.size(), len2 = s2.size();
        std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

        d[0][0] = 0;
        for(unsigned int i = 1; i <= len1; ++i) d[i][0] = i;
        for(unsigned int i = 1; i <= len2; ++i) d[0][i] = i;

        for(unsigned int i = 1; i <= len1; ++i)
            for(unsigned int j = 1; j <= len2; ++j)
            {
                d[i][j] = min(min(d[i - 1][j] + 1, d[i][j - 1] + 1), d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
            }
        return d[len1][len2];
    }

};


#endif //DATAGENERATOR_UTILS_H
