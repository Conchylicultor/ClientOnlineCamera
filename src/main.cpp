#include <iostream>
#include <string>

#include "exchangemanager.h"

using namespace std;


int main()
{
    ExchangeManager &exchangeManager = ExchangeManager::getInstance();

    while (1) {
        exchangeManager.loop();
    }
    return 0;
}
