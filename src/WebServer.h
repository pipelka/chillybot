#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class WebServer : public AsyncWebServer {
public:

    explicit WebServer(uint16_t port);

protected:

    void getStatusJSON(const JsonObject& json);

    void setOverrideJSON(const JsonObject& json);

    void getNetworksJSON(AsyncWebServerRequest* request);

    void handleUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t *data, size_t len, bool final);

private:

    char m_lastModified[30]{0};
};
