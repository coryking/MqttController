//
// Created by Cory King on 11/4/17.
//

#ifndef MQTTCONTROLLER_MQTTCONTROLLER_H
#define MQTTCONTROLLER_MQTTCONTROLLER_H

#include <PubSubClient.h>
#include <Task.h>
#include <WiFiClient.h>
#include <Syslog.h>


// how long to wait for MQTT to settle before trying to connect
#define MQTT_CONNECT_WAIT 6000

enum MQTT_STATE {
    Disconnecting,
    Disconnected,
    Waiting,
    Connecting,
    Connected
};

class MqttController : public Task {
public:
    typedef std::function<void(void)> ReconnectCallback;

    MqttController(String mqttServer, uint16_t mqttPort) : mqttServer(mqttServer), mqttPort(mqttPort), Task(MsToTaskTime(100)) {
        getRandomClientId();

    }
    virtual void OnUpdate(uint32_t deltaTime);

    void setReconnectCallback(const ReconnectCallback reconnectCallback);


protected:
    PubSubClient *client;
    virtual void OnDoUpdate(uint32_t deltaTime) = 0;
    void publish(String &topic, const char* value);
    void subscribe(String& topic);
    virtual void setSubscriptions();
    virtual void mqttCallback(char* topic, byte* payload, unsigned int length);

private:
    String mqttServer;
    uint16_t mqttPort = 1883;
    WiFiClient wifiClient;
    ulong timeWaitStarted = 0;
    char espClientId[20] = {0};
    char espTopicId[20] = {0};
    void getRandomClientId();
    void makePubSubClient();
    MQTT_STATE mqttState = Disconnected;
    ReconnectCallback reconnectCallback;


    void toWaiting();

};


#endif //MQTTCONTROLLER_MQTTCONTROLLER_H
