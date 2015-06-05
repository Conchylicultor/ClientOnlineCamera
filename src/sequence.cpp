#include "sequence.h"

#include <fstream>

Sequence::Sequence(const string &nameSequence)
{
    string sequenceId;

    // Extract the person name
    string::size_type separatorPos = nameSequence.find(':');

    if(separatorPos == nameSequence.size()-1)// Case where label == ""
    {
        sequenceId = nameSequence.substr(0, separatorPos);
        name = sequenceId;
    }
    else if(separatorPos != string::npos)// If the person has been labelised
    {
        sequenceId = nameSequence.substr(0, separatorPos);
        name = nameSequence.substr(separatorPos + 1, string::npos);
    }
    else // No label (so the name is the sequence id)
    {
        sequenceId = nameSequence;
        name = nameSequence;
    }

    cout << name << "| |" << sequenceId << "|"<<endl;

    // Load all image id
    // Otherwise, simply add the image to the current person

    // Extract the sequence image list
    ifstream imgListFile("/home/etienne/__A__/Dev/Reidentification/Data/Traces/" + sequenceId + "_list.txt");
    if(!imgListFile.is_open())
    {
        cout << "Error: Cannot open the images list of sequence " << sequenceId << endl;
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

