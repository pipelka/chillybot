#include "MQTTClient.h"
#include "Status.h"

MQTTClient::MQTTClient() : client(wifiClient) {
}

void MQTTClient::begin(const char* server, int port, const String& clientid) {
    if(server == nullptr || server[0] == 0) {
        return;
    }

    client.setServer(server, port);

    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
        String value;
        auto& status = Status::instance();

        for (unsigned int i = 0; i < length; i++) {
            value += (char)payload[i];
        }

        MQTTClient::logMsg(topic, value.c_str(), false);

        if(strcmp(topic, createTopic(topicOverride)) == 0) {
            status.overrideTemperature(value.toFloat(), false);
        }

        if(strcmp(topic, createTopic(topicOverrideEnabled)) == 0) {
            status.overrideEnabled((value == "true"), false);
        }
    });

    this->deviceId = clientid;

    Serial.print(F("[mqtt] "));
    Serial.print(server);
    Serial.print(F(":"));
    Serial.print(port);
    Serial.print(F(" (device: "));
    Serial.print(this->deviceId);
    Serial.println(")");
}

void MQTTClient::loop() {
    if(!client.connected() && WiFi.isConnected()) {
        if(client.connect(deviceId.c_str())) {
            Serial.println("[mqtt] connected to server.");

            client.subscribe(createTopic(topicOverride));
            client.subscribe(createTopic(topicOverrideEnabled));
        }
    }

    auto& status = Status::instance();

    if(status.overrideChanged()) {
        publishOverride(status.overrideEnabled(), status.overrideTemperature());
        status.clearOverrideChanged();
    }

    client.loop();
}

void MQTTClient::publishTemp(float currentTemp, float targetTemp) {
    if(!client.connected()) {
        return;
    }

    if(currentTemp != 0) {
        snprintf(msg, sizeof(msg), "%.1f", currentTemp);

        const char* topic = createTopic(topicCurrent);
        client.publish(topic, msg);

        if(prevTemp != currentTemp) {
            logMsg(topic, msg);
            prevTemp = currentTemp;
        }
    }

    if(targetTemp != 0) {
        snprintf(msg, sizeof(msg), "%.1f", targetTemp);

        const char* topic = createTopic(topicTarget);
        client.publish(topic, msg, true);

        if(prevTarget != targetTemp) {
            logMsg(topic, msg);
            prevTarget = targetTemp;
        }
    }
}

void MQTTClient::publishOverride(bool enable, float temp) {
    if(!client.connected()) {
        return;
    }

    snprintf(msg, sizeof(msg), "%s", enable ? "true" : "false");

    const char* topic = createTopic(topicOverrideEnabled);
    logMsg(topic, msg);
    client.publish(topic, msg);

    if(temp > 0) {
        snprintf(msg, sizeof(msg), "%.1f", temp);

        const char* topic = createTopic(topicOverride);
        logMsg(topic, msg);
        client.publish(topic, msg);
    }
}
void MQTTClient::logMsg(const char* topic, const char* msg, bool out) {
    Serial.print(F("[mqtt] "));
    Serial.print(out ? "<< " : ">> ");
    Serial.print(topic);
    Serial.print(F(" : "));
    Serial.println(msg);
}

const char* MQTTClient::createTopic(const String& topicTemplate) {
    static char buffer[64];
    snprintf(buffer, sizeof(buffer) - 1, topicTemplate.c_str(), deviceId.c_str());

    return buffer;
}
