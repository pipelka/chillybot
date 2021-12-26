#pragma once

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WString.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <functional>

class BrewfatherDevice {
public:

    BrewfatherDevice();

    virtual ~BrewfatherDevice() = default;

    void setName(const String& name);

    void setTemperatureUnit(const String& unit);

    void setTemperature(float temperature);

    void setExternalTemperature(float temperature);

    int send();

    void clear();

    void attach(int interval = 20);

    void attach(int interval, std::function<void(void)> f);

    void detach();

    bool active() const;

    void begin(const String& name, const String& url, const String& temperatureUnit = "C");

    void loop();

    bool needAttach() const;

private:

    HTTPClient mHttp;

    WiFiClient mClient;

    String mName;

    String mUrl;

    DynamicJsonDocument mBuffer;

    Ticker mTicker;
};
