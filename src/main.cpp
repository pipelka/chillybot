#include <Arduino.h>
//#include <ArduinoOTA.h>
#include <Ticker.h>
#include <OneWire.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <LittleFS.h>

#include "SensorBus.h"
#include "MQTTClient.h"
#include "BrewfatherDevice.h"
#include "BrewfatherBatchSync.h"

#include "WebServer.h"
#include "Status.h"
#include "Configuration.h"
#include "device.h"

OneWire oneWire(PIN_TEMP);
SensorBus sensors(&oneWire);
MQTTClient client;
Ticker blinker;
Ticker dataLogger;
Ticker logic;
BrewfatherDevice device;
BrewfatherBatchSync sync;
WebServer webServer(80);
Configuration& config = Configuration::instance();
bool apMode = false;

unsigned long lastCoolTime{0};
unsigned long lastHeatTime{0};
unsigned long runTime{60};

float hysteresisCooling{0.3};
float hysteresisHeating{0.6};

void setup() {
    Serial.begin(9600);

    pinMode(PIN_RELAY_COOL, OUTPUT);
    digitalWrite(PIN_RELAY_COOL, SWITCH_OFF);

#ifdef HEATING_ENABLED
    pinMode(PIN_RELAY_HEAT, OUTPUT);
    digitalWrite(PIN_RELAY_HEAT, SWITCH_OFF);
#endif

    // initial delay (use LED to signal startup)

    pinMode(PIN_LED_WIFI, OUTPUT);
    digitalWrite(PIN_LED_WIFI, LED_OFF);

    Serial.println();
    Serial.println(F("----------------------------------"));
    Serial.println(F(" chillyBot temperature controller"));
    Serial.println(F("----------------------------------"));
    Serial.println();
    Serial.println(F("starting ..."));
    Serial.println();

    Serial.println(F("[fs] checking filesystem ..."));

    if(!LittleFS.begin()) {
        Serial.println(F("[fs] formatting filesystem ..."));
        if(LittleFS.format()) {
            Serial.println(F("[fs] done."));
        }
        else {
            Serial.println(F("[fs] error formatting the filesystem."));
        }
    }
    else {
        Serial.println(F("[fs] filesystem ready."));
    }

    // configuration
    config.load();

    const char* ssid = config.getString("wifi", "ssid");
    if(ssid == nullptr || ssid[0] == 0 || ssid[0] == ' ') {
        apMode = true;
    }

    // start sensors
    Serial.println("[sensors] initializing");
    sensors.begin();

    // set wifi parameters
    if(apMode) {
        Serial.println("[wifi] starting AP mode");

        config.set("wifi", "ssid", "chillybot");
        config.set("wifi", "psk", "chillybot");
        config.set("wifi", "dhcp", true);
        config.set("brewfather", "deviceid", "chillybot");

        WiFi.mode(WIFI_AP);
        WiFi.softAP(
            config.getString("wifi", "ssid"),
            config.getString("wifi", "psk")
        );
    }
    else {
        WiFi.mode(WIFI_STA);
        WiFi.begin(
            config.getString("wifi", "ssid"),
            config.getString("wifi", "psk")
        );
    }

    const char* deviceid = config.getString("brewfather", "deviceid");

    // initialize brewfather stream device
    device.begin(
        config.getString("brewfather", "deviceid"),
        config.getString("brewfather", "logurl")
    );

    // mqtt client
    client.begin(
        config.getString("mqtt", "server"),
        config.getInteger("mqtt", "port"),
        deviceid
    );

    // MDNS
    MDNS.begin(deviceid);

    // configure ota updates
    //ArduinoOTA.setHostname(deviceid);
    //ArduinoOTA.begin();

    // schedule mqtt data publishing
    dataLogger.attach_scheduled(2, []() {
        auto temperature = Status::round(sensors.getTemp(0));
        auto targetTemperature = Status::instance().targetTemperature();

        device.setTemperature(temperature);
        device.setExternalTemperature(targetTemperature);

        if(temperature != 0 && targetTemperature) {
            device.loop();
        }

        client.publishTemp(temperature, targetTemperature);
    });

    // initialize brewfather batch sync
    sync.begin(
        deviceid,
        config.getString("brewfather", "apiuser"),
        config.getString("brewfather", "apikey")
    );

    // configure ntp / timezone
    const char* timezone = config.getString("ntp", "timezone");
    const char* ntpserver = config.getString("ntp", "server");

    Serial.print(F("[ntp] server: "));
    Serial.println(ntpserver);
    Serial.print(F("[ntp] tz: "));
    Serial.println(timezone);
    configTime(timezone, ntpserver);

    // webserver
    webServer.begin();
}

void loop() {
    // AP mode
    if(apMode) {
        if(!blinker.active()) {
            blinker.attach_ms_scheduled(125, []() {
                digitalWrite(PIN_LED_WIFI, !digitalRead(PIN_LED_WIFI));
            });
        }
    }
    // connection phase blinking (wifi connection)
    else if(!WiFi.isConnected() && !blinker.active()) {
        Serial.println("[wifi] initializing network connection ...");
        blinker.attach_ms_scheduled(250, []() {
            digitalWrite(PIN_LED_WIFI, !digitalRead(PIN_LED_WIFI));
        });
    }
    else if(blinker.active() && WiFi.isConnected()) {
        digitalWrite(PIN_LED_WIFI, LED_ON);
        Serial.print(F("[wifi] connection up ("));
        Serial.print(WiFi.SSID());
        Serial.print(F(" / channel: "));
        Serial.print(WiFi.channel());
        Serial.println(")");

        blinker.detach();
    }
    else if(WiFi.isConnected()) {
        digitalWrite(PIN_LED_WIFI, LED_ON);
    }

    // tick all subsystems
    //ArduinoOTA.handle();
    sensors.loop();
    client.loop();
    sync.loop();

    // temperature control
    auto targetTemperature = Status::instance().targetTemperature();
    auto temp = Status::round(sensors.getTemp(0));

    // set temp
    Status::instance().currentTemperature(temp);

    // core logic (extremly sophisticated)

    bool coolState{false};
    bool heatState{false};

    if(targetTemperature != 0) {
        coolState = (temp > targetTemperature + hysteresisCooling); 
        heatState = (temp < targetTemperature - hysteresisHeating); 
    }

    if(coolState && lastCoolTime == 0) {
        lastCoolTime = millis();
        digitalWrite(PIN_RELAY_COOL, SWITCH_ON);
    }
    else if(!coolState && lastCoolTime != 0 && millis() - lastCoolTime >= runTime * 1000) {
        lastCoolTime = 0;
        digitalWrite(PIN_RELAY_COOL, SWITCH_OFF);
    }

#ifdef HEATING_ENABLED
    if(heatState && lastHeatTime == 0) {
        lastHeatTime = millis();
        digitalWrite(PIN_RELAY_HEAT, SWITCH_ON);
    }
    else if(!heatState && lastHeatTime != 0 && millis() - lastHeatTime >= runTime * 1000) {
        lastHeatTime = 0;
        digitalWrite(PIN_RELAY_HEAT, SWITCH_OFF);
    }
#endif

    delay(10);
}
