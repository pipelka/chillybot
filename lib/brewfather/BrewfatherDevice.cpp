#include <BrewfatherDevice.h>

static float round1(float value) {
    return (int)(value * 10.0f + 0.5f) / 10.0f;
}

BrewfatherDevice::BrewfatherDevice() : mBuffer(128) {
}

void BrewfatherDevice::begin(const String& name, const String& url, const String& temperatureUnit) {
    if(name.isEmpty() || url.isEmpty()) {
        return;
    }

    mBuffer[F("name")] = name;
    mName = name;
    mUrl = url;
    setTemperatureUnit(temperatureUnit);

    Serial.print(F("[stream] url: "));
    Serial.println(url);
}

void BrewfatherDevice::setName(const String& name) {
    mName = name;
}

void BrewfatherDevice::setTemperatureUnit(const String& unit) {
    mBuffer["temp_unit"] = unit;
}

void BrewfatherDevice::setTemperature(float temperature) {
    mBuffer[F("temp")] = round1(temperature);
}

void BrewfatherDevice::setExternalTemperature(float temperature) {
    mBuffer[F("ext_temp")] = round1(temperature);
}

void BrewfatherDevice::clear() {
    mBuffer.clear();
    mBuffer[F("name")] = mName;
}

int BrewfatherDevice::send() {
    if(!mHttp.begin(mClient, mUrl)) {
        Serial.print(F("[stream] error connecting to: "));
        Serial.println(mUrl);
        return -1;
    }

    Serial.println(F("[stream] sending device data ..."));

    String output;
    serializeJson(mBuffer, output);

    Serial.print(F("[stream] json: "));
    Serial.println(output);

    mHttp.addHeader(F("Content-Type"), F("application/json"));

    int status = mHttp.POST(output);

    if(status == HTTP_CODE_OK) {
        const String& response = mHttp.getString();
        Serial.print(F("[stream] response: "));
        Serial.println(response);
    }

    mHttp.end();

    Serial.print(F("[stream] done. status = "));
    Serial.println(status);

    return status;
}

void BrewfatherDevice::attach(int interval) {
    mTicker.attach_scheduled(interval * 60, [this]() {
        this->send();
    });

    this->send();
}

void BrewfatherDevice::attach(int interval, std::function<void(void)> f) {
    mTicker.attach_scheduled(interval * 60, f);
}

void BrewfatherDevice::detach() {
    mTicker.detach();
}

bool BrewfatherDevice::active() const {
    return mTicker.active();
}

bool BrewfatherDevice::needAttach() const {
    return WiFi.isConnected() && !active();
}

void BrewfatherDevice::loop() {
    if(mName.isEmpty()) {
        return;
    }

    if(needAttach()) {
        attach();
    }
}
