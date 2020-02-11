#include "WeatherClient.h"

#include <ESP8266HTTPClient.h>
#include <base64.h>
#include "WeatherConfig.h"
#include "Log.h"

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

void WeatherClient::SetLongitudeAndLatitude(const String &lat, const String &lon)
{
    this->lat = lat;
    this->lon = lon;
}

int WeatherClient::GetForecast5Weather()
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

    String body = http.getString();
    Log::Info("body:");
    Log::Info(body.c_str());
    return httpCode;
}
