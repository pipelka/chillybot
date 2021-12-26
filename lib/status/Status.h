#pragma once
#include <WString.h>

class Status {
protected:

    Status() = default;

    virtual ~Status() = default;

public:

    static Status& instance();

    inline float currentTemperature() const {
        return m_currentTemperature;
    }

    inline void currentTemperature(float temp) {
        m_currentTemperature = temp;
    }

    inline float targetTemperature() const {
        return overrideEnabled() ? overrideTemperature() : recipeTemperature();
    }

    inline float recipeTemperature() const {
        return m_recipeTemperature;
    }

    inline void recipeTemperature(float temp) {
        m_recipeTemperature = Status::round(temp);
    }

    inline bool overrideEnabled() const {
        return m_overrideEnabled;
    }

    inline void overrideEnabled(bool enable, bool changed = true) {
        if(m_overrideEnabled == enable) {
            return;            
        }

        m_overrideEnabled = enable;
        m_overrideChanged = changed;
    }

    inline float overrideTemperature() const {
        return m_overrideTemperature;
    }

    void overrideTemperature(float temp, bool changed = true) {
        if(temp == m_overrideTemperature) {
            return;
        }

        m_overrideTemperature = Status::round(temp);
        m_overrideChanged = changed;
    }

    inline bool overrideChanged() const {
        return m_overrideChanged;
    }

    inline void clearOverrideChanged() {
        m_overrideChanged = false;
    }

    inline const String& batchName() const {
        return m_batchName;
    }

    void batchName(const String& name) {
        m_batchName = name;
    }

    inline const int batchNumber() const {
        return m_batchNumber;
    }

    inline void batchNumber(int number) {
        m_batchNumber = number;
    }

    static float round(float value) {
        return (int)(value * 10.0f + 0.5f) / 10.0f;
    }

private:

    float m_currentTemperature{0.0f};

    float m_recipeTemperature{0.0f};

    bool m_overrideEnabled{false};

    float m_overrideTemperature{0.0f};

    bool m_overrideChanged{false};

    int m_batchNumber;

    String m_batchName;

};
