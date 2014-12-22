#ifndef FEATURESMANAGER_H
#define FEATURESMANAGER_H

#include <iostream>
#include <string>
#include <vector>

#include "sequence.h"

using namespace std;

/**
 * @brief The FeaturesManager class read the sequences, extract the images and send the features to the network
 */
class FeaturesManager
{
public:
    FeaturesManager();
    ~FeaturesManager();
    void sendNext();

private:
    vector<Sequence> listSequences;

    unsigned int currentSequenceId;

    float *arrayToSend;
};

#endif // FEATURESMANAGER_H
