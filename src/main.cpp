#include <iostream>
#include <string>

#include "exchangemanager.h"
#include "featuresmanager.h"

using namespace std;


int main()
{
    ExchangeManager &exchangeManager = ExchangeManager::getInstance();

    FeaturesManager featuresManager;

    while (1) {
        featuresManager.sendNext();
        exchangeManager.loop();
    }
    return 0;
}
