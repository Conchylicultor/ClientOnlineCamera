#include "featuresmanager.h"

#include <functional>
#include <fstream>
#include "exchangemanager.h"
#include "features.h"

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

    // Start with the first sequence
    currentSequenceId = 0;

    // Allocation of the array
    arrayToSend = nullptr;
}

FeaturesManager::~FeaturesManager()
{
    delete arrayToSend;
}

void FeaturesManager::sendNext()
{
    if(ExchangeManager::getInstance().getIsActive() &&
            !ExchangeManager::getInstance().getIsLocked() &&
            currentSequenceId < listSequences.size())
    {
        Sequence &currentSequence = listSequences.at(currentSequenceId);
        cout << "Sequence: " << currentSequence.getName() << endl;
        if(currentSequence.getListImageIds().size() < MIN_SEQUENCE_SIZE)
        {
            cout << "Sequence ignored (too short)"<< endl;
        }
        else
        {
            vector<FeaturesElement> listCurrentSequenceFeatures;

            // Selection of some images of the sequence (we don't send the entire sequence)
            for(unsigned int i = 0 ; i < MIN_SEQUENCE_SIZE ; ++i)
            {
                listCurrentSequenceFeatures.push_back(FeaturesElement());
                int index = i * ((currentSequence.getListImageIds().size()-1)/MIN_SEQUENCE_SIZE); // Linear function (0,0) to (MIN_SEQUENCE_SIZE, listImageIds().size()-1)
                Features::computeFeature("../../Data/Traces/" + currentSequence.getListImageIds().at(index), listCurrentSequenceFeatures.back());
            }

            // Fill the array
            size_t arrayToSendSize = 1; // Offset
            Features::computeArray(arrayToSend, arrayToSendSize, listCurrentSequenceFeatures);
            arrayToSend[0] = std::hash<std::string>()(currentSequence.getName()); // Hashcode used as id

            // Send the features over the network
            ExchangeManager::getInstance().publishFeatures(arrayToSendSize*sizeof(float),arrayToSend);
        }
        ++currentSequenceId;
    }
}
