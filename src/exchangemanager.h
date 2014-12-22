#ifndef EXCHANGEMANAGER_H
#define EXCHANGEMANAGER_H

#include <list>
#include <string>

#include "mosquittopp.h"

struct ConnectedClient
{
    unsigned int id;
    unsigned int version_number;
};

class ExchangeManager : public mosqpp::mosquittopp
{
public:
    static ExchangeManager &getInstance();
    bool getIsActive() const;

private:
    ExchangeManager();
    ~ExchangeManager();

    // Assure C++ compatibility, handle errors, remove unused parameter
    void publish(const std::string &topic, int payloadlen=0, const void *payload=NULL, int qos=0, bool retain=false);
    void subscribe(const std::string &sub, int qos=0);

    void on_message(const struct mosquitto_message *message);
    void onProtocolVersion(const struct mosquitto_message *message); // Received protocol version from the main client

    // Unique client id
    int clientId;
    // True if we are on the same protocol version that the main client and that exchanges can be active
    bool isActive;
};

#endif // EXCHANGEMANAGER_H
