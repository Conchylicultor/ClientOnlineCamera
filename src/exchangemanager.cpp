#include "exchangemanager.h"

#include <unistd.h>
#include <iostream>
#include <string>
#include "opencv2/opencv.hpp"

using namespace std;

const string protocolVersionTopic = "reid_client/protocol_version";
const string newCamTopic = "cam_clients/new_connection";
const string removeCamTopic = "cam_clients/remove_connection";
const string dataCamTopic = "cam_clients/data";

const unsigned int version_protocol = 1;

ExchangeManager &ExchangeManager::getInstance()
{
    static ExchangeManager instance;
    return instance;
}

ExchangeManager::ExchangeManager() : mosqpp::mosquittopp(),
    clientId(0),
    isActive(false),
    isLocked(false),
    midExit(0),
    exitSend(false)
{
    // Initialise the library
    mosqpp::lib_init();

    // Loading the configuration
    string brokerAdress;

    cv::FileStorage fileConfig("../config.yml", cv::FileStorage::READ);
    if(!fileConfig.isOpened())
    {
        cout << "Error: cannot open the configuration file" << endl;
        exit(0);
    }

    fileConfig["brokerIp"] >> brokerAdress;

    if(!fileConfig["clientId"].empty())
    {
        fileConfig["clientId"] >> clientId;
    }
    else
    {
        cout << "Warning: Try to attribute an id to the camera client" << endl;
        clientId = ::getpid();
    }

    fileConfig.release();

    // Creating a client instance
    // /!\ Warning: Each client must have a UNIQUE id !!!
    this->reinitialise(string("CamClient_" + std::to_string(clientId)).c_str(), true);

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
    cout << "Try connecting the client " << clientId << " to the brocker..." << endl;
    result = this->connect(brokerAdress.c_str());
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
    // Send the last will
    this->publish(removeCamTopic,
                  sizeof(int),
                  &clientId,
                  2,
                  false,
                  &midExit);

    // Wait that the message has been send
    while(!exitSend)
    {
        loop();
    }

    // Disconnect properly
    this->disconnect();

    mosqpp::lib_cleanup(); // End of use of this library
}

void ExchangeManager::publishFeatures(int payloadlen, const void *payload)
{
    isLocked = true;
    publish(dataCamTopic, payloadlen, payload, 2);
}

bool ExchangeManager::getIsActive() const
{
    return isActive;
}

bool ExchangeManager::getIsLocked() const
{
    return isLocked;
}

void ExchangeManager::publish(const string &topic, int payloadlen, const void *payload, int qos, bool retain, int *mid)
{
    // Publish
    int result = mosqpp::mosquittopp::publish(mid, topic.c_str(), payloadlen, payload, qos, retain);

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
    if(protocolVersionReceived != version_protocol)
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

void ExchangeManager::on_publish(int mid)
{
    // If the exit message has been send, we can exit properly
    if(mid == midExit)
    {
        exitSend = true;
    }
    // Unlock exchange for the next publication
    else
    {
        std::cout << "Publication done !!!" << std::endl;
        isLocked = false;
    }
}
