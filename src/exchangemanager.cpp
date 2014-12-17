#include "exchangemanager.h"

#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;
#define ADDRESS     "192.168.200.11"

const string protocolVersionTopic = "reid_client/protocol_version";
const string newCamTopic = "cam_clients/new_connection";
const string removeCamTopic = "cam_clients/remove_connection";

const unsigned int version_protocol = 1;

ExchangeManager &ExchangeManager::getInstance()
{
    static ExchangeManager instance;
    return instance;
}

ExchangeManager::ExchangeManager() : mosqpp::mosquittopp(),
    isActive(false)
{
    // Initialise the library
    mosqpp::lib_init();

    clientId = ::getpid();

    // Creating a client instance
    // /!\ Warning: Each client must have a UNIQUE id !!!
    this->reinitialise(std::to_string(clientId).c_str(), true);

    // Configure the will before the connection
    int result = this->will_set(removeCamTopic.c_str(),
                                sizeof(int),
                                &clientId,
                                2);
    switch (result)
    {
    case MOSQ_ERR_SUCCESS:
        break;
    case MOSQ_ERR_INVAL:
        cout << "Error testimony : invalid parameters" << endl;
        break;
    case MOSQ_ERR_NOMEM:
        cout << "Error testimony : out of memory" << endl;
        break;
    default:
        cout << "Error testimony : ???" << endl;
        break;
    }

    // Connect the client to the broker.
    // Please indicate the right IP address or server name
    result = this->connect(ADDRESS);
    // Check the result
    switch (result)
    {
    case MOSQ_ERR_SUCCESS:
        cout << "Connection successful" << endl;
        break;
    case MOSQ_ERR_INVAL:
        cout << "Error connection : invalid parameters" << endl;
        break;
    case MOSQ_ERR_ERRNO:
        cout << "Error connection : server error" << endl;
        break;
    default:
        cout << "Error connection : ???" << endl;
        break;
    }

    // Subscription to the main client (see if we use the same protocol)
    this->subscribe(protocolVersionTopic, 2);
}

ExchangeManager::~ExchangeManager()
{
    this->publish(removeCamTopic,
                  sizeof(int),
                  &clientId,
                  2);

    // Disconnect properly
    this->disconnect();

    mosqpp::lib_cleanup(); // End of use of this library
}

void ExchangeManager::publish(const string &topic, int payloadlen, const void *payload, int qos, bool retain)
{
    // Publish
    int result = mosqpp::mosquittopp::publish(NULL, topic.c_str(), payloadlen, payload, qos, retain);

    // Check the result
    switch (result) {
    case MOSQ_ERR_SUCCESS:
        cout << "Publication successful to : " << topic << endl;
        break;
    case MOSQ_ERR_INVAL:
        cout << "Error publication : invalid parameters" << endl;
        break;
    case MOSQ_ERR_NOMEM:
        cout << "Error publication : out of memory" << endl;
        break;
    case MOSQ_ERR_NO_CONN:
        cout << "Error publication : client not connected to a broker" << endl;
        break;
    case MOSQ_ERR_PROTOCOL:
        cout << "Error publication : protocol error" << endl;
        break;
    case MOSQ_ERR_PAYLOAD_SIZE:
        cout << "Error publication : payloadlen is too large" << endl;
        break;
    default:
        cout << "Error publication : ???" << endl;
        break;
    }
}

void ExchangeManager::subscribe(const string &sub, int qos)
{
    int result = mosqpp::mosquittopp::subscribe(NULL, sub.c_str(), qos);

    // Check the result
    switch (result) {
    case MOSQ_ERR_SUCCESS:
        cout << "Subscription successful : " << sub << endl;
        break;
    case MOSQ_ERR_INVAL:
        cout << "Error publication : invalid parameters" << endl;
        break;
    case MOSQ_ERR_NOMEM:
        cout << "Error publication : out of memory" << endl;
        break;
    case MOSQ_ERR_NO_CONN:
        cout << "Error publication : client not connected to a broker" << endl;
        break;
    default:
        cout << "Error publication : ???" << endl;
        break;
    }
}

void ExchangeManager::on_message(const mosquitto_message *message)
{
    cout << "Message received : " << message->topic << endl;
    if(message->topic == protocolVersionTopic)
    {
        onProtocolVersion(message);
    }
}

void ExchangeManager::onProtocolVersion(const mosquitto_message *message)
{
    unsigned int protocolVersionReceived = *((unsigned int*)message->payload);
    if(protocolVersionReceived > version_protocol)
    {
        cout << "Error: protocol incompatible, desactivation of the client" << endl;
        isActive = false;
    }
    else
    {
        // Activation of the client
        isActive = true;
        // Send response
        ConnectedClient client;
        client.id = clientId;
        client.version_number = version_protocol;
        this->publish(newCamTopic,
                      sizeof(ConnectedClient),
                      &client,
                      2);
    }
}
