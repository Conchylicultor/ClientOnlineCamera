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
    void sendNext();

private:
    vector<Sequence> listSequences;
};

#endif // FEATURESMANAGER_H
