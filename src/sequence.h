#ifndef SEQUENCE_H
#define SEQUENCE_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Sequence
{
public:
    Sequence(const string &nameSequence);

    string getName() const;

    void addImageId(const string &newId);
    vector<string> getListImageIds() const;

    string getCamInfoId() const;
    void setCamInfoId(const string &value);

private:
    string name;
    string camInfoId;
    vector<string> listImageIds;
};

#endif // SEQUENCE_H
