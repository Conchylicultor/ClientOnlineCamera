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
        // The associated camera information file
        else if(line.find("_cam") != std::string::npos)
        {
            currentSequence->setCamInfoId(line);
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

            size_t arrayToSendSize = 0; // Offset
            if(sizeof(size_t)/sizeof(float) != 2) // Depending of the computer architecture
            {
                cout << "Error: wrong architecture 2!=" << sizeof(size_t)/sizeof(float) << ", cannot send the size_t" << endl;
            }
            arrayToSendSize += 2; // Hashcode sequence name
            arrayToSendSize += 2; // Hashcode camera name
            arrayToSendSize += 2; // Entrance and exit time
            arrayToSendSize += 8; // Entrance and exit vectors

            Features::computeArray(arrayToSend, arrayToSendSize, listCurrentSequenceFeatures);

            // Add offset values to the array
            computeAddInfo(arrayToSend, currentSequence);

            // Send the features over the network
            ExchangeManager::getInstance().publishFeatures(arrayToSendSize*sizeof(float),arrayToSend);
        }
        ++currentSequenceId;
    }
}

void FeaturesManager::computeAddInfo(float *&array, const Sequence &sequence)
{
    if(sizeof(float) != sizeof(int)) // For casting
    {
        cout << "Error: convertion float<->int impossible" << endl;
        exit(0);
    }

    int value = 0; // Used as intermediate for bit copy into float

    FileStorage fileTraceCam("../../Data/Traces/" + sequence.getCamInfoId() + ".yml", FileStorage::READ);
    if(!fileTraceCam.isOpened())
    {
        cout << "Error: Cannot open the additional information file for the sequence " << sequence.getCamInfoId() << endl;
        exit(0);
    }

    // Save the sequence name id
    size_t hashCode = 0;
    unsigned int mostSignificantBits = 0;
    unsigned int leastSignificantBits = 0;

    hashCode = std::hash<std::string>()(sequence.getName()); // Hashcode used as id

    leastSignificantBits = hashCode & 0xFFFFFFFF;
    mostSignificantBits = hashCode >> 32 & 0xFFFFFFFF;
    array[0] = reinterpret_cast<float&>(leastSignificantBits);
    array[1] = reinterpret_cast<float&>(mostSignificantBits);

    // Save the cam id
    hashCode = std::hash<std::string>()(fileTraceCam["camId"]);

    leastSignificantBits = hashCode & 0xFFFFFFFF;
    mostSignificantBits = hashCode >> 32 & 0xFFFFFFFF;
    array[2] = reinterpret_cast<float&>(leastSignificantBits);
    array[3] = reinterpret_cast<float&>(mostSignificantBits);

    // Save the entrance and exit time
    value = fileTraceCam["beginDate"];
    array[4] = reinterpret_cast<float&>(value);
    value = fileTraceCam["endDate"];
    array[5] = reinterpret_cast<float&>(value);

    // Save the entrance and exit direction
    FileNode nodeVector;

    nodeVector = fileTraceCam["entranceVector"];
    array[6] = static_cast<float>(nodeVector["x1"]);
    array[7] = static_cast<float>(nodeVector["y1"]);
    array[8] = static_cast<float>(nodeVector["x2"]);
    array[9] = static_cast<float>(nodeVector["y2"]);

    nodeVector = fileTraceCam["exitVector"];
    array[10] = static_cast<float>(nodeVector["x1"]);
    array[11] = static_cast<float>(nodeVector["y1"]);
    array[12] = static_cast<float>(nodeVector["x2"]);
    array[13] = static_cast<float>(nodeVector["y2"]);

    fileTraceCam.release();
}
