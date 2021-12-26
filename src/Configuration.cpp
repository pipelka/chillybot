#include "Configuration.h"
#include <LittleFS.h>

Configuration::Configuration() : m_config(1024) {
    m_config[F("wifi")].to<JsonObject>();
    m_config[F("ntp")].to<JsonObject>();
    m_config[F("brewfather")].to<JsonObject>();
    m_config[F("mqtt")].to<JsonObject>();

    set("wifi", "dhcp", true);
    set("ntp", "server", "at.pool.ntp.org");
    set("ntp", "timezone", "CET-1CEST,M3.5.0/02,M10.5.0/03");
    set("brewfather", "deviceid", "chillybot");
}

Configuration& Configuration::instance() {
    static Configuration o;
    return o;
}

DynamicJsonDocument& Configuration::get() {
    return m_config;
}

bool Configuration::load() {
    Serial.println(F("[config] loading configuration from fs ..."));

    File file = LittleFS.open(F("/configuration.json"), "r");
    if(!file) {
        Serial.println(F("[config] configuration not found."));
        return false;
    }

    deserializeJson(m_config, file);
    file.close();

    serializeJsonPretty(m_config, Serial);
    Serial.println(F("[config] done."));

    return true;
}

bool Configuration::save() {
    Serial.println(F("[config] writing configuration ..."));

    File file = LittleFS.open(F("/configuration.json"), "w+");
    if(!file) {
        Serial.println(F("[config] unable to open file !"));
        return false;
    }

    serializeJson(m_config, file);
    Serial.println(F("[config] done."));
    file.close();

    return true;
}
