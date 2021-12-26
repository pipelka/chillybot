#include <BrewfatherBatchSync.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <Status.h>

void BrewfatherBatchSync::begin(const String& deviceid, const String& apiUser, const String& apiKey) {
    if(deviceid.isEmpty() || apiUser.isEmpty() || apiKey.isEmpty()) {
        return;
    }

    mDeviceId = deviceid;
    for(auto& c : mDeviceId) {
        c = ::toUpperCase(c);
    }

    if(!apiUser.isEmpty() && !apiKey.isEmpty()) {
        String userpass = apiUser + ":" + apiKey;
        mAuth = base64::encode(userpass, false);
    }
}

void BrewfatherBatchSync::loop() {
    if(mDeviceId.isEmpty()) {
        return;
    }

    if(time(nullptr) < 100000 || !WiFi.isConnected()) {
        return;
    }

    auto now = millis();

    if(now - mLast < 5 * 60 * 1000 && mLast != 0) {
        return;
    }

    if(fetchTargetTemp()) {
        mLast = now;
    }
}

bool BrewfatherBatchSync::fetchTargetTemp() {
    if(!WiFi.isConnected() || mAuth.isEmpty()) {
        return true;
    }

    Serial.println(F("[sync] fetching target temp ..."));
    String url = F("https://api.brewfather.app/v1/batches?include=recipe.fermentation.steps,recipe.name,devices.stream&complete=False&status=Fermenting");

    mClient.setInsecure();

    // timestamp in ms
    int64_t now = time(nullptr) * 1000;
    Serial.print(F("[sync] now: "));
    Serial.println(now);

    if(!http.begin(mClient, url)) {
        Serial.print(F("[sync] unable to open url: "));
        Serial.println(url);
        return false;
    }

    http.setTimeout(10000);
    http.useHTTP10();
    http.setAuthorization(mAuth.c_str());
    http.addHeader(F("Content-Type"), F("application/json"));

    int status = http.GET();

    if(status != 200) {
        Serial.print(F("[sync] unable to fetch data. status = "));
        Serial.println(status);
        http.end();
        return false;
    }

    DynamicJsonDocument doc(5*1024);
    deserializeJson(doc, http.getStream());
    for(const JsonObject& batch: doc.as<JsonArray>()) {
        const JsonArray& items = batch["devices"]["stream"]["items"];

        int batchNumber = batch["batchNo"];

        Serial.print(F("[sync] checking batch #"));
        Serial.print(batchNumber);
        Serial.print(F(" - "));
        Serial.print(items.size());
        Serial.println(F(" devices ..."));

        // search for deviceid in items
        bool found = false;
        int j{0};

        for(const JsonObject& item: items) {
            String device = item[F("name")].isNull() ? item[F("key")] : item[F("name")];
            device.toUpperCase();

            //bool enabled = item["enabled"];

            Serial.print(F("[sync] device "));
            Serial.print(++j);
            Serial.print(F(": "));
            Serial.println(device);
            //Serial.print(F(" - "));
            //Serial.println(enabled == true ? F("enabled") : F("disabled"));

            if(/*enabled == true &&*/ device == mDeviceId) {
                found = true;
            }
            else {
                Serial.print("[sync] ");
                serializeJson(item, Serial);
                Serial.println();
            }
        }

        // skip batch if device is not included
        if(!found) {
            continue;
        }

        // get batch number / name
        const String& batchName = batch["recipe"]["name"];

        Status::instance().batchNumber(batchNumber);
        Status::instance().batchName(batchName);

        Serial.print(F("[sync] batch #"));
        Serial.print(batchNumber);
        Serial.print(F(" - "));
        Serial.println(batchName);

        // iterate through fermentation steps
        const JsonArray& steps = batch["recipe"]["fermentation"]["steps"];

        for(const JsonObject& step: steps) {
            int64_t start = step["actualTime"];
            int64_t end = start + ((int64_t)step["stepTime"]) * 24 * 60 * 60 * 1000;

            int temp = step["stepTemp"];

            Serial.print(F("[sync] range #"));
            Serial.print(j+1);
            Serial.print(F(" : "));
            Serial.print(start);
            Serial.print(F(" - "));
            Serial.print(end);
            Serial.print(F(" - "));
            Serial.print(temp);
            Serial.println("C");

            if(start <= now && now < end) {
                Status::instance().recipeTemperature(temp);
                Serial.print(F("[sync] target temp: "));
                Serial.println(temp);
                break;
            }
        }
    }

    http.end();
    return true;
}
