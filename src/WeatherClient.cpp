#include "WeatherClient.h"

#include <ESP8266HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>
#include "WeatherConfig.h"
#include "Log.h"

StaticJsonDocument<10000> doc;

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

void WeatherClient::SetLongitudeAndLatitude(const String &lat, const String &lon)
{
    this->lat = lat;
    this->lon = lon;
}

int WeatherClient::GetCurrentWeather(void (*fn)(time_t t, const char *main))
{
    String uri = createWeatherUri(CURRENT_WEATHER_URL, API_KEY, lat, lon);
    HTTPClient http;
    http.begin(uri);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.sendRequest("GET");
    if (httpCode != 200) {
        http.end();
        return httpCode;
    }

    String json = http.getString();
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        String log = "deserializeJson() error :";
        log += err.c_str();
        Log::Error(log.c_str());
        http.end();
        return -1;
    }

    const char  *main = doc["weather"][0]["main"];
    const time_t dt   = doc["dt"];
    const int    hour = unixtimeToHour(dt);

    String log = __FUNCTION__;
    log += "(),";
    log += dt;
    log += ",";
    log += hour;
    log += ",";
    log += main;
    Log::Info(log.c_str());

    (*fn)(dt, main);
    http.end();
    return httpCode;
}

int WeatherClient::GetForecast5Weather(void (*fn)(time_t t, const char *main))
{
    String uri = createWeatherUri(FORECAST5_WEATHER_URL, API_KEY, lat, lon);
    HTTPClient http;
    http.begin(uri);
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.sendRequest("GET");
    if (httpCode != 200) {
        http.end();
        return httpCode;
    }

    String json = http.getString();
    // FORECAST5 returns too much body, then remove json
    json.remove(2048);
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        // FORECAST5 returns too much body, then ignore IncompleteInput
        String log = "deserializeJson() error :";
        log += err.c_str();
        if (err.code() != err.IncompleteInput) {
            http.end();
            Log::Error(log.c_str());
            return -1;
        }
        Log::Info(log.c_str());
    }

    for (int i=0; i<5; i++) {
        const char  *dt_txt = doc["list"][i]["dt_txt"];
        const time_t dt     = doc["list"][i]["dt"];
        const char  *main   = doc["list"][i]["weather"][0]["main"];
        const int    hour   = unixtimeToHour(dt);

        String log = __FUNCTION__;
        log += "(),";
        log += dt;
        log += ",";
        log += hour;
        log += ",";
        log += dt_txt;
        log += ",";
        log += main;
        Log::Info(log.c_str());

        (*fn)(dt, main);
    }
    http.end();
    return httpCode;
}
