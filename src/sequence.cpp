#include "sequence.h"

Sequence::Sequence(const string &nameSequence) :
    name(nameSequence)
{
}

string Sequence::getName() const
{
    return name;
}

void Sequence::addImageId(const string &newId)
{
    listImageIds.push_back(newId);
}

vector<string> Sequence::getListImageIds() const
{
    return listImageIds;
}

string Sequence::getCamInfoId() const
{
    return camInfoId;
}

void Sequence::setCamInfoId(const string &value)
{
    camInfoId = value;
}

