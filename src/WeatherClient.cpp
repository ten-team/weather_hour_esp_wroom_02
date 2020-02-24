#include "WeatherClient.h"

#include <ESP8266HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>
#include <log/Log.h>
#include "WeatherConfig.h"

static WiFiClient client;
static HTTPClient http;
static StaticJsonDocument<8192> doc;

static String createWeatherUri(const char *base, const char *api, const String &lat, const String &lon)
{
    String uri = base;
    uri += "?APPID=";
    uri += api;
    uri += "&lat=";
    uri += lat;
    uri += "&lon=";
    uri += lon;
    return uri;
}

int unixtimeToHour(time_t t)
{
    return t / 60 / 60 % 24;
}

int unixtimeToJstHour(time_t t)
{
    return (unixtimeToHour(t) + 9) % 12;
}

int unixtimeToMinute(time_t t)
{
    return t / 60 % 60;
}

void WeatherClient::setLongitudeAndLatitude(const String &lat, const String &lon)
{
    this->lat = lat;
    this->lon = lon;
}

int WeatherClient::getCurrentWeather(void (*fn)(time_t t, const char *main))
{
    doc.clear();
    String uri = createWeatherUri(CURRENT_WEATHER_URL, API_KEY, lat, lon);
    Log::Info(uri.c_str());
    http.begin(client, uri);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.sendRequest("GET");
    if (httpCode != 200) {
        String log = "http.sendRequest(GET) returns :";
        log += httpCode;
        http.end();
        return httpCode;
    }

    String json = http.getString();
    DynamicJsonDocument doc(1024);
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        String log = "deserializeJson() error :";
        log += err.c_str();
        Log::Error(log.c_str());
        Log::Error(json.c_str());
        http.end();
        return -1;
    }

    const char  *main = doc["weather"][0]["main"];
    const time_t dt   = doc["dt"];
    const int    hour = unixtimeToHour(dt);
    const int    min  = unixtimeToMinute(dt);

    String log = __FUNCTION__;
    log += "(),";
    log += dt;
    log += ",";
    log += hour;
    log += ":";
    log += min;
    log += ",";
    log += main;
    if (main == 0) {
        Log::Error(log.c_str());
        Log::Error(json.c_str());
        http.end();
        return -1;
    }
    Log::Info(log.c_str());

    (*fn)(dt, main);
    http.end();
    return httpCode;
}

int WeatherClient::getForecast5Weather(void (*fn)(int index, time_t t, const char *main))
{
    doc.clear();
    String uri = createWeatherUri(FORECAST5_WEATHER_URL, API_KEY, lat, lon);
    Log::Info(uri.c_str());
    http.begin(client, uri);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.sendRequest("GET");
    if (httpCode != 200) {
        String log = "http.sendRequest(GET) returns :";
        log += httpCode;
        Log::Error(log.c_str());
        http.end();
        return httpCode;
    }

    String json = http.getString();
    // FORECAST5 returns too much body, then remove json
    json.remove(4096);
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        // FORECAST5 returns too much body, then ignore IncompleteInput
        String log = "deserializeJson() error :";
        log += err.c_str();
        if (err.code() != err.IncompleteInput) {
            Log::Error(log.c_str());
            Log::Error(json.c_str());
            http.end();
            return -1;
        }
        Log::Info(log.c_str());
    }

    for (int i=0; i<8; i++) {
        const char  *dt_txt = doc["list"][i]["dt_txt"];
        const time_t dt     = doc["list"][i]["dt"];
        const char  *main   = doc["list"][i]["weather"][0]["main"];
        const int    hour   = unixtimeToHour(dt);
        const int    min    = unixtimeToMinute(dt);

        String log = __FUNCTION__;
        log += "(),";
        log += dt;
        log += ",";
        log += hour;
        log += ":";
        log += min;
        log += ",";
        log += dt_txt;
        log += ",";
        log += main;
        if (main == 0) {
            Log::Error(log.c_str());
            Log::Error(json.c_str());
            http.end();
            return -1;
        }
        Log::Info(log.c_str());

        (*fn)(i, dt, main);
    }
    http.end();
    return httpCode;
}
