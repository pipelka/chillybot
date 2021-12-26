#pragma once
#include <WString.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

class BrewfatherBatchSync {
public:

    BrewfatherBatchSync() = default;

    virtual ~BrewfatherBatchSync() = default;

    void begin(const String& deviceid, const String& apiUser, const String& apiKey);

    void loop();

protected:

    bool fetchTargetTemp();

private:

    BearSSL::WiFiClientSecure mClient;

    HTTPClient http;

    String mDeviceId;

    String mAuth;

    unsigned int mLast{0};
};
