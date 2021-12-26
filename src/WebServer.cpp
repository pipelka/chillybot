#include "WebServer.h"
#include "web/index.html.gz.h"
#include <AsyncJson.h>
#include <LittleFS.h>
#include <Status.h>
#include "Configuration.h"

AsyncWebServerResponse* createChunkedMemoryResponse(AsyncWebServerRequest *request, const String& contentType, const uint8_t* content, size_t contentLength) {
    auto response = request->beginChunkedResponse(F("text/html"), [=](uint8_t* buffer, size_t maxLen, size_t index) -> size_t {
        size_t chunkSize = maxLen;

        if(index + chunkSize > contentLength) {
            chunkSize = contentLength - index;
        }

        if(chunkSize == 0) {
            return 0;
        }

        memcpy_P(buffer, content + index, chunkSize);

        return chunkSize;
    });

    response->addHeader(F("Content-Encoding"), F("gzip"));
    return response;
};

AsyncWebServerResponse* createMemoryResponse(AsyncWebServerRequest *request, const String& contentType, const uint8_t* content, size_t contentLength) {
    auto response = request->beginResponse_P(200, contentType, content, contentLength);
    response->addHeader(F("Content-Encoding"), F("gzip"));
    return response;
};

WebServer::WebServer(uint16_t port) : AsyncWebServer(port) {
    rewrite("/", "/index.html");

    on("/rest/status", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto response = new AsyncJsonResponse();

        const JsonObject& root = response->getRoot();
        getStatusJSON(root);

        response->setLength();
        request->send(response);
    });

    on("/rest/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
        auto response = new AsyncJsonResponse();

        auto root = response->getRoot();
        const JsonObject& o = Configuration::instance().get().as<JsonObject>();

        root.set(o);

        response->setLength();
        request->send(response);
    });

    on("/rest/listnetworks", HTTP_GET, [this](AsyncWebServerRequest *request) {
        getNetworksJSON(request);
    });

    addHandler(new AsyncCallbackJsonWebHandler(F("/rest/reset"), [this](AsyncWebServerRequest *request, JsonVariant &json) {
        request->send(200);

        Serial.println(F("Rebooting ..."));
        ESP.restart();
    }));

    addHandler(new AsyncCallbackJsonWebHandler(F("/rest/setoverride"), [this](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject& jsonObj = json.as<JsonObject>();
        setOverrideJSON(jsonObj);
        request->send(200);
    }));

    addHandler(new AsyncCallbackJsonWebHandler(F("/rest/setconfig"), [this](AsyncWebServerRequest *request, JsonVariant &json) {
        const JsonObject& jsonObj = json.as<JsonObject>();

        auto& instance = Configuration::instance();
        instance.get().set(jsonObj);
        instance.save();
        request->send(200);

        Serial.println(F("Rebooting ..."));
        LittleFS.end();
        ESP.restart();
    }));

    on("/index.html", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if(m_lastModified[0] == 0) {
            auto now = time(nullptr);
            struct tm* tm = localtime(&now);
            strftime(m_lastModified, sizeof(m_lastModified) - 1, "%c", tm);
        }

        if (request->header(F("If-Modified-Since")).equals(m_lastModified)) {
            request->send(304);
        }
        else {
            AsyncWebServerResponse* response = createMemoryResponse(request, F("text/html"), index_html_gz, index_html_gz_len);
            response->addHeader(F("Last-Modified"), m_lastModified);
            request->send(response);
        }
    });

    onNotFound([](AsyncWebServerRequest *request) {
        request->send(404);
    });

    onFileUpload([this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        this->handleUpload(request, filename, index, data, len, final);
    });
}

void WebServer::getStatusJSON(const JsonObject &json) {
    struct ip_info info{};
    FSInfo fsinfo{};

    if(!LittleFS.info(fsinfo)) {
        Serial.print(F("[fs] error getting info on filesystem"));
    }

    const JsonObject& system = json[F("system")].to<JsonObject>();

    system[F("heap")] = ESP.getFreeHeap();
    system[F("chipid")] = String(ESP.getChipId(), HEX);
    system[F("cpu")] = ESP.getCpuFreqMHz();
    system[F("availsize")] = ESP.getFreeSketchSpace();
    system[F("availspiffs")] = fsinfo.totalBytes - fsinfo.usedBytes;
    system[F("spiffssize")] = fsinfo.totalBytes;
    system[F("uptime")] = millis();

    if (WiFi.getMode() == WIFI_AP) {
        struct softap_config conf{};
        wifi_softap_get_config(&conf);
        wifi_get_ip_info(SOFTAP_IF, &info);

        system[F("ssid")] = String(reinterpret_cast<char *>(conf.ssid));
        system[F("dns")] = WiFi.softAPIP().toString();
        system[F("mac")] = WiFi.macAddress();
        system[F("channel")] = F("N/A");
    }
    else {
        struct station_config conf{};
        wifi_station_get_config(&conf);
        wifi_get_ip_info(STATION_IF, &info);

        system[F("ssid")] = String(reinterpret_cast<char *>(conf.ssid));
        system[F("dns")] = WiFi.dnsIP().toString();
        system[F("mac")] = WiFi.macAddress();
        system[F("channel")] = WiFi.channel();
        system[F("rssi")] = WiFi.RSSI();
        system[F("bssid")] = WiFi.BSSIDstr();
    }

    system[F("ip")] = IPAddress(info.ip.addr).toString();
    system[F("gateway")] = IPAddress(info.gw.addr).toString();
    system[F("netmask")] = IPAddress(info.netmask.addr).toString();

    const JsonObject& temp = json[F("temp")].to<JsonObject>();
    temp[F("current")] = Status::instance().currentTemperature();
    temp[F("target")] = Status::instance().targetTemperature();

    const JsonObject& override = json[F("override")].to<JsonObject>();
    override[F("enabled")] = Status::instance().overrideEnabled();
    override[F("target")] = Status::instance().overrideTemperature();

    const JsonObject& batch = json[F("batch")].to<JsonObject>();
    batch[F("number")] = Status::instance().batchNumber();
    batch[F("name")] = Status::instance().batchName();
}

void WebServer::setOverrideJSON(const JsonObject &json) {
    Status::instance().overrideTemperature(json[F("target")]);
    Status::instance().overrideEnabled(json[F("enabled")]);
}

void WebServer::getNetworksJSON(AsyncWebServerRequest *request) {
    int rc = WiFi.scanComplete();

    if(rc == WIFI_SCAN_RUNNING) {
        request->send(304);
        return;
    }

    WiFi.scanNetworksAsync([=](int num) {
        auto response = new AsyncJsonResponse();
        const JsonObject& root = response->getRoot();

        root[F("num")] = num;
        const JsonArray &scan = root.createNestedArray(F("list"));

        for (uint8_t i = 0; i < num; ++i) {
            const JsonObject &item = scan.createNestedObject();

            item[F("ssid")] = WiFi.SSID(i);
            item[F("bssid")] = WiFi.BSSIDstr(i);
            item[F("rssi")] = WiFi.RSSI(i);
            item[F("channel")] = WiFi.channel(i);
            item[F("enctype")] = WiFi.encryptionType(i);
        }

        response->setLength();
        request->send(response);
    });
}

void WebServer::handleUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    File file;

    if(!index){
        Serial.printf("UploadStart: %s\n", filename.c_str());
        file = LittleFS.open(filename, "w");
    }
    else {
        file = LittleFS.open(filename, "a");
    }

    file.write(data, len);

    if(final) {
        Serial.printf("UploadEnd: %s, %u B\n", filename.c_str(), index + len);
    }

    file.close();
}
