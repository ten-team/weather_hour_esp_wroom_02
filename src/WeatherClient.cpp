#include "WeatherClient.h"

#include <ESP8266HTTPClient.h>
#include <base64.h>
#include <ArduinoJson.h>
#include "WeatherConfig.h"
#include "Log.h"

StaticJsonDocument<10000> doc;
String json;

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

    json = http.getString();
    deserializeJson(doc, json);

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

    json = http.getString();
    deserializeJson(doc, json);
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
    return httpCode;
}
