#pragma once
#include <OneWire.h>
#include <DallasTemperature.h>

#define DEVICE_CONVERSION_NOT_COMPLETE -256

class SensorBus {
public:

    SensorBus(OneWire* oneWire);

    void begin();

    void loop();

    uint8_t getDeviceCount();

    float getTemp(uint8_t index, bool celsius = true);

    void setMeasureInterval(unsigned int intervalMs);

    bool isParasitePowerMode();
    
protected:

    bool requestTemperatures();

private:

    DallasTemperature m_sensors;

    DeviceAddress m_thermometer[8];

    float m_temperature[8]{0.0f};

    uint32_t m_lastTempRequest{0};

    unsigned int m_measureIntervalMs = 1000;

    bool m_requesting = false;
};
