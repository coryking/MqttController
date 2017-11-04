//
// Created by Cory King on 11/4/17.
//

#include "MqttController.h"


void MqttController::publish(String &topic, const char* value) {
    if(this->mqttState == Connected) {
        syslog.logf(LOG_INFO, "p: [%s] '%s'\n", value, topic.c_str());
        this->client->publish(topic.c_str(), value);
        yield();
        this->client->loop();
        yield();
    } else {
        syslog.logf(LOG_INFO, "ig: [%s] '%s'\n", value, topic.c_str());
    }

}

void MqttController::subscribe(String &topic) {
    if(this->mqttState == Connected) {
        syslog.logf(LOG_INFO, "sub '%s'\n", topic.c_str());
        this->client->subscribe(topic.c_str());
    } else {
        syslog.logf(LOG_INFO, "ignore '%s'\n", topic.c_str());

    }
}

void MqttController::setReconnectCallback(const MqttController::ReconnectCallback reconnectCallback) {
    MqttController::reconnectCallback = reconnectCallback;
}

void MqttController::getRandomClientId() {
    sprintf(espTopicId, "ESP_%06X", ESP.getChipId());
    sprintf(espClientId, "%s_%04X", espTopicId, random(0,65535));
    Serial.printf("CID:[%s]",espClientId);
    syslog.logf(LOG_INFO, "CID:[%s]",espClientId);
}

void MqttController::makePubSubClient() {
    if(client != NULL)
        delete client;

    client = new PubSubClient();
    client->setClient(wifiClient);
    client->setServer(mqttServer, mqttPort);
    client->setCallback([this](char* topic, byte* payload, unsigned int length) {
        this->mqttCallback(topic,payload,length);
    });
}


void MqttController::OnUpdate(uint32_t deltaTime) {
    Task::OnUpdate(deltaTime);

    if(this->client == NULL || this->mqttState == Disconnecting)
    {
        this->wifiClient.stop();
        syslog.logf(LOG_WARNING, "To Mqtt Disconnected");
        this->mqttState = Disconnected;
    }

    if(this->mqttState == Disconnected) {
        this->getRandomClientId();
        this->makePubSubClient();
        toWaiting();
    }

    if(this->mqttState == Waiting) {
        if(millis() > this->timeWaitStarted + MQTT_CONNECT_WAIT)
        {
            syslog.logf(LOG_WARNING, "To Mqtt Connecting...");
            this->mqttState = Connecting;

        }
    }

    if(this->mqttState == Connecting) {
        if(this->client->connect(espClientId)) {
            syslog.logf(LOG_WARNING, "To Mqtt Connected...");
            this->mqttState = Connected;

            // ... and resubscribe
            this->setSubscriptions();
            if(reconnectCallback != NULL)
                reconnectCallback();
        } else {
            this->toWaiting();
        }
    }

    if(this->mqttState == Connected) {
        if(!this->client->loop())
        {
            this->mqttState = Disconnecting;
        }
    }

    this->OnDoUpdate(deltaTime);
}

void MqttController::toWaiting() {
    syslog.logf(LOG_WARNING, "To Mqtt Waiting...");
    timeWaitStarted = millis();
    mqttState = Waiting;
}
