#include "featuresmanager.h"

#include <fstream>
#include "exchangemanager.h"

static const unsigned int MIN_SEQUENCE_SIZE = 5; // Otherwise, the sequence is ignored

bool replace(std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

FeaturesManager::FeaturesManager()
{
    // Loading the lists of sequences

    ifstream fileListPersons("../../Data/Traces/traces.txt");
    if(!fileListPersons.is_open())
    {
        cout << "Error: Impossible to load the trace file" << endl;
        exit(0);
    }

    // /!\ Warning: No verification on the file

    Sequence *currentSequence = nullptr;
    for(string line; std::getline(fileListPersons, line); )
    {
        // If new group of image
        if(line.find("-----") != std::string::npos)
        {
            replace(line, "----- ", "");
            replace(line, " -----", "");

            listSequences.push_back(Sequence(line));
            currentSequence = &listSequences.back();
        }
        // Otherwise, simply add the image to the current person
        else
        {
            currentSequence->addImageId(line);
        }
    }

    fileListPersons.close();

    currentSequenceId = 0;
}

void FeaturesManager::sendNext()
{
    if(ExchangeManager::getInstance().getIsActive() && currentSequenceId < listSequences.size())
    {
        cout << "Sequence: " << listSequences.at(currentSequenceId).getName() << endl;
        if(listSequences.at(currentSequenceId).getListImageIds().size() < MIN_SEQUENCE_SIZE)
        {
            cout << "Sequence ignored (too short)"<< endl;
        }
        else
        {

        }
        ++currentSequenceId;
    }
}
