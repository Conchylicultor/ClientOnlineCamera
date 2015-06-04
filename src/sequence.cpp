#include "sequence.h"

#include <fstream>

Sequence::Sequence(const string &nameSequence) :
    name(nameSequence)
{
    // Load all image id
    // Otherwise, simply add the image to the current person

    // Extract the sequence image list
    ifstream imgListFile("/home/etienne/__A__/Dev/Reidentification/Data/Traces/" + name + "_list.txt");
    if(!imgListFile.is_open())
    {
        cout << "Error: Cannot open the images list of sequence " << name << endl;
        return;
    }

    // Read entire file
    for(string newId ; std::getline(imgListFile, newId) ; )
    {
        listImageIds.push_back(newId);
    }

    imgListFile.close();
}

string Sequence::getName() const
{
    return name;
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

