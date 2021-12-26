#pragma once
#include <ArduinoJson.h>

class Configuration {
protected:

    Configuration();

    virtual ~Configuration() = default;

public:

    static Configuration& instance();

    DynamicJsonDocument& get();

    inline const char* getString(const char* section, const char* key) {
        return m_config[section][key];
    }

    template<class T>
    inline void set(const char* section, const char* key, T value) {
        m_config[section][key] = value;
    }

    inline int getInteger(const char* section, const char* key) {
        return m_config[section][key];
    }

    bool load();

    bool save();

private:

    DynamicJsonDocument m_config;

};
