#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "Log.h"
#include "Config.h"
#include "ConfigServer.h"
#include "WeatherConfig.h"
#include "WeatherClient.h"

#define MODE_PIN          16
#define NEO_PIXEL_PIN     5
#define NUM_OF_NEO_PIXELS_PER_HOUR 3
#define NUM_OF_NEO_PIXELS (NUM_OF_NEO_PIXELS_PER_HOUR * 12)
#define NEO_PIXEL_STOCK_0 0

Adafruit_NeoPixel pixels        = Adafruit_NeoPixel(NUM_OF_NEO_PIXELS,
                                                    NEO_PIXEL_PIN,
                                                    NEO_GRB + NEO_KHZ800);

const uint32_t BLACK_COLOR      = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t ERROR_COLOR      = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t WAITING_COLOR    = Adafruit_NeoPixel::Color(255, 255, 51);
const uint32_t CONFIG_COLOR     = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t EXISTS_COLOR     = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t NOT_EXSITS_COLOR = Adafruit_NeoPixel::Color(255, 0, 0);

const uint32_t WEATHER_COLOR_CLEAR   = Adafruit_NeoPixel::Color(255, 170, 0);
const uint32_t WEATHER_COLOR_CLOUDS  = Adafruit_NeoPixel::Color(170, 170, 170);
const uint32_t WEATHER_COLOR_RAIN    = Adafruit_NeoPixel::Color(0, 65, 255);
const uint32_t WEATHER_COLOR_SNOW    = Adafruit_NeoPixel::Color(242, 242, 255);
const uint32_t WEATHER_COLOR_UNKNOWN = Adafruit_NeoPixel::Color(255, 0, 0);

const int WEB_ACCESS_INTERVAL   = (5 * 60 * 1000);

WeatherClient weather;

static void showError()
{
    while (true) {
        for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
            pixels.setPixelColor(i, ERROR_COLOR);
        }
        pixels.show();
        delay(1000);

        for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
            pixels.setPixelColor(i, BLACK_COLOR);
        }
        pixels.show();
        delay(1000);
    }
}

static void reconnectWifi()
{
    String ssid;
    String pass;
    String lat;
    String lon;
    if (!Config::ReadWifiConfig(ssid, pass, lat, lon)) {
        Log::Error("Faild to read config.");
        showError();
    }
    weather.SetLongitudeAndLatitude(lat, lon);
    // If forget mode(WIFI_STA), mode might be WIFI_AP_STA.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Log::Info("WiFi connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        pixels.setPixelColor(NEO_PIXEL_STOCK_0, WAITING_COLOR);
        pixels.show();
        Log::Debug(".");
        delay(500);

        pixels.setPixelColor(NEO_PIXEL_STOCK_0, BLACK_COLOR);
        pixels.show();
        Log::Debug(".");
        delay(500);
    }

    Log::Info("WiFi connected.");
    String log = "IP address: ";
    IPAddress ip = WiFi.localIP();
    log += ip[0]; log += ".";
    log += ip[1]; log += ".";
    log += ip[2]; log += ".";
    log += ip[3];
    Log::Info(log.c_str());
}

static uint32_t weather2color(const String &weather)
{
    // see http://www.jma.go.jp/jma/kishou/info/colorguide/120524_hpcolorguide.pdf
    if      (weather == "Thunderstorm") { return WEATHER_COLOR_RAIN; }
    else if (weather == "Drizzle")      { return WEATHER_COLOR_RAIN; }
    else if (weather == "Rain")         { return WEATHER_COLOR_RAIN; }
    else if (weather == "Snow")         { return WEATHER_COLOR_SNOW; }
    else if (weather == "Atmosphere")   { return WEATHER_COLOR_CLOUDS; }
    else if (weather == "Clear")        { return WEATHER_COLOR_CLEAR; }
    else if (weather == "Clouds")       { return WEATHER_COLOR_CLOUDS; }
    else if (weather == "Extreme")      { return WEATHER_COLOR_UNKNOWN; }
    else if (weather == "Additional")   { return WEATHER_COLOR_UNKNOWN; }
    else                                { return WEATHER_COLOR_UNKNOWN; }
}

static void setHourWeather(int hour, uint32_t color)
{
    pixels.setPixelColor(hour * NUM_OF_NEO_PIXELS_PER_HOUR,     color);
    pixels.setPixelColor(hour * NUM_OF_NEO_PIXELS_PER_HOUR + 1, color);
    pixels.setPixelColor(hour * NUM_OF_NEO_PIXELS_PER_HOUR + 2, color);
}

static uint32_t getHourWeather(int hour)
{
    pixels.getPixelColor(hour * NUM_OF_NEO_PIXELS_PER_HOUR);
}

static void fnForecast5Weather(time_t t, const char *main)
{
    int hour = unixtimeToHour(t);
    int jst12Hour = (hour + 9) % 12;
    uint32_t color = weather2color(String(main));
    if (getHourWeather(jst12Hour) != BLACK_COLOR) {
        return;
    }
    setHourWeather(jst12Hour,     color);
    setHourWeather(jst12Hour + 1, color);
    setHourWeather(jst12Hour + 2, color);
}

static void fnCurrentWeather(time_t t, const char *main)
{
    int hour = unixtimeToHour(t);
    int jst12Hour = (hour + 9) % 12;
    uint32_t color = weather2color(String(main));
    int remain = 3 - (jst12Hour % 3);
    for (int i=0; i<remain; i++) {
        setHourWeather(jst12Hour + i, color);
    }
}

static void showExistState()
{
    for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
        pixels.setPixelColor(i, BLACK_COLOR);
    }

    weather.GetForecast5Weather(fnForecast5Weather);
    weather.GetCurrentWeather(fnCurrentWeather);

    pixels.show();
}

void setup()
{
    Serial.begin(115200);
    Serial.println("");

    pixels.begin();
    pixels.setBrightness(124);

    pinMode(MODE_PIN, INPUT);

    if (!Config::Initialize()) {
        Log::Error("Faild to execute SPIFFS.begin().");
        showError();
    }

    if (digitalRead(MODE_PIN) == LOW) {
        Log::Info("Detected Config mode.");
        pixels.setPixelColor(NEO_PIXEL_STOCK_0, CONFIG_COLOR);
        pixels.show();
        ConfigServer::Start();
        // can not reach here.
    }
    Log::Info("Detected Normal mode.");
    reconnectWifi();
}

void loop()
{
    showExistState();
    delay(WEB_ACCESS_INTERVAL);
}
