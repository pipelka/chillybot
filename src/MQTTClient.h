#pragma once
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

class MQTTClient {
public:

    MQTTClient();

    virtual ~MQTTClient() = default;

    void begin(const char* server, int port, const String& clientid);

    void loop();

    inline bool connected() {
        return client.connected();
    }

    void publishTemp(float currentTemp, float targetTemp = 0.0f);

    void publishOverride(bool enable, float temp);

protected:

    static void logMsg(const char* topic, const char* msg, bool out = true);

    const char* createTopic(const String& topicTemplate);

private:

    WiFiClient wifiClient;

    PubSubClient client;
 
    String deviceId;

    float prevTemp{0.0f};

    float prevTarget{0.0f};

    char msg[10]{0};

    const char* topicCurrent = "/sensors/%s/temp_current";
    const char* topicTarget = "/sensors/%s/temp_target";
    const char* topicOverride = "/sensors/%s/temp_override";
    const char* topicOverrideEnabled = "/sensors/%s/temp_override_enabled";
};
