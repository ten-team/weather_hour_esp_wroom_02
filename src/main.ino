#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include <log/Log.h>
#include "Config.h"
#include "ConfigServer.h"
#include "WeatherConfig.h"
#include "WeatherClient.h"
#include "WeatherData.h"

#define MODE_PIN                   16
#define NEO_PIXEL_PIN              5
#define NUM_OF_NEO_PIXELS_PER_HOUR 3
#define SEC_PER_LED_INTERVAL       (60 * 60 / NUM_OF_NEO_PIXELS_PER_HOUR)
#define SEC_PER_HARF_DAY           (60 * 60 * 12)
#define NUM_OF_NEO_PIXELS          (NUM_OF_NEO_PIXELS_PER_HOUR * 12)
#define NEO_PIXEL_STOCK_0          0
#define BREATHING_BRIGHTNESS_OFFSET 16
#define BREATHING_INTERVAL          80

Adafruit_NeoPixel pixels        = Adafruit_NeoPixel(NUM_OF_NEO_PIXELS,
                                                    NEO_PIXEL_PIN,
                                                    NEO_GRB + NEO_KHZ800);

const uint32_t BLACK_COLOR      = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t ERROR_COLOR      = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t WAITING_COLOR    = Adafruit_NeoPixel::Color(255, 255, 51);
const uint32_t CONFIG_COLOR     = Adafruit_NeoPixel::Color(255, 255, 255);

const uint32_t NOW_COLOR             = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t WEATHER_COLOR_CLEAR   = Adafruit_NeoPixel::Color(255, 170, 0);
const uint32_t WEATHER_COLOR_CLOUDS  = Adafruit_NeoPixel::Color(170, 170, 170);
const uint32_t WEATHER_COLOR_RAIN    = Adafruit_NeoPixel::Color(0, 65, 255);
const uint32_t WEATHER_COLOR_UNKNOWN = Adafruit_NeoPixel::Color(255, 0, 0);

const int WEB_ACCESS_INTERVAL   = (5 * 60 * 1000);
const int SHOW_INTERVAL         = (10 * 1000);
const int SHOW_WEATHER_TIME     = (2400);
const int WEBAPI_RETRY_MAX      = 10;

WeatherClient weatherClient;
WeatherData   weatherData;

#define PRINT_FREE_RAM() printFreeRam(__FILE__, __LINE__)

static void printFreeRam(const char *file, int line) {
    String log = file;
    log += ",";
    log += line;
    log += ",Free heap memory,";
    log += ESP.getFreeHeap();
    Log::Info(log.c_str());
}

static void showError(int times)
{
    for (int i=0; times<0 || i<times; i++) {
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

static int getDelayTimeWhenConnectingWifi(int index)
{
    int d = (NUM_OF_NEO_PIXELS / 2 - index);
    if (d < 0) {
        d = -d;
    }
    d = NUM_OF_NEO_PIXELS / 2 - d;
    return d * d / 3;
}

static void showConnectingWifi()
{
    clearLeds();
    pixels.show();
    delay(100);

    for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
        pixels.setPixelColor(i, WAITING_COLOR);
        pixels.show();
        int d = getDelayTimeWhenConnectingWifi(i);
        delay(d);
    }

    for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
        pixels.setPixelColor(i, BLACK_COLOR);
        pixels.show();
        int d = getDelayTimeWhenConnectingWifi(i);
        delay(d);
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
        showError(-1);
    }
    weatherClient.setLongitudeAndLatitude(lat, lon);
    // If forget mode(WIFI_STA), mode might be WIFI_AP_STA.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Log::Info("WiFi connecting...");
    while (WiFi.status() != WL_CONNECTED) {
        showConnectingWifi();
        Log::Debug(".");
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
    else if (weather == "Mist")         { return WEATHER_COLOR_RAIN; }
    else if (weather == "Snow")         { return WEATHER_COLOR_RAIN; }
    else if (weather == "Atmosphere")   { return WEATHER_COLOR_CLOUDS; }
    else if (weather == "Clear")        { return WEATHER_COLOR_CLEAR; }
    else if (weather == "Clouds")       { return WEATHER_COLOR_CLOUDS; }
    else if (weather == "Extreme")      { return WEATHER_COLOR_UNKNOWN; }
    else if (weather == "Additional")   { return WEATHER_COLOR_UNKNOWN; }
    else                                { return WEATHER_COLOR_UNKNOWN; }
}

static void setWeatherColorOne(uint32_t t, uint32_t color)
{
    int hour      = unixtimeToJstHour(t);
    int hourIndex = hour * NUM_OF_NEO_PIXELS_PER_HOUR;
    int min       = unixtimeToMinute(t);
    int minIndex  = min * 60 / SEC_PER_LED_INTERVAL;
    int index     = (hourIndex + minIndex) % NUM_OF_NEO_PIXELS;
    pixels.setPixelColor(index, color);
}

static void setWeatherColor(uint32_t from, uint32_t to, uint32_t color)
{
    from = from - from % SEC_PER_LED_INTERVAL;
    for (uint32_t t=from; t<to; t+=(SEC_PER_LED_INTERVAL)) {
        setWeatherColorOne(t, color);
    }
}

static void fnForecast5Weather(int index, time_t t, const char *main)
{
    if (t == 0) {
        Log::Error("Fails to decode json in fnForecast5Weather()");
        return;
    }
    WeatherDataOne &data = weatherData.getForecastWeather(index);
    data.setTime(t);
    data.setWeather(main);
}

static void fnCurrentWeather(time_t t, const char *main)
{
    if (t == 0) {
        Log::Error("Fails to decode json in fnCurrentWeather()");
        return;
    }
    WeatherDataOne &data = weatherData.getCurrentWeather();
    data.setTime(t);
    data.setWeather(main);
}

static void clearLeds()
{
    for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
        pixels.setPixelColor(i, BLACK_COLOR);
    }
}

static uint32_t colorWithBrightness(uint32_t color, int brightness)
{
    uint32_t r = ((color >> 16) & 0xFF) * brightness / 256;
    uint32_t g = ((color >>  8) & 0xFF) * brightness / 256;
    uint32_t b = ((color >>  0) & 0xFF) * brightness / 256;
    return (r << 16) + (g << 8) + (b << 0);
}

static void setCurrentTimeLed(WeatherDataOne &current, int brightness)
{
    uint32_t color = colorWithBrightness(NOW_COLOR, brightness);
    setWeatherColor(current.getTime(), current.getTime()+1, color);
}

static void setWeatherLed(WeatherDataOne &w1, WeatherDataOne &w2, int brightness)
{
    uint32_t color = weather2color(w1.getWeather().c_str());
    setWeatherColor(w1.getTime(), w2.getTime(), colorWithBrightness(color, brightness));
}

static void breathingOut()
{
    uint32_t colors[NUM_OF_NEO_PIXELS];
    for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
        colors[i] = pixels.getPixelColor(i);
    }

    for (int b=256; b>=0; b-=BREATHING_BRIGHTNESS_OFFSET) {
        for (int i=0; i<NUM_OF_NEO_PIXELS; i++) {
            pixels.setPixelColor(i, colorWithBrightness(colors[i], b));
        }
        pixels.show();
        delay(BREATHING_INTERVAL);
    }
}

static void showWeather(WeatherDataOne &c,
                        WeatherDataOne &f0, WeatherDataOne &f1,
                        WeatherDataOne &f2, WeatherDataOne &f3)
{
    WeatherDataOne c12 = c;
    c12.setTime(c.getTime() + SEC_PER_HARF_DAY);
    for (int i=0; i<256; i+=BREATHING_BRIGHTNESS_OFFSET) {
        setWeatherLed(c,  f0, i);
        setWeatherLed(f0, f1, i);
        setWeatherLed(f1, f2, i);
        setWeatherLed(f2, f3, i);
        setWeatherLed(f3, c12, i);
        setCurrentTimeLed(c, i);
        pixels.show();
        delay(BREATHING_INTERVAL);
    }
}

static void breathingWeatherGradually(WeatherDataOne &c,
                                      WeatherDataOne &w1, WeatherDataOne &w2)
{
    bool interlock = false;
    if (c.getTime() >= w1.getTime() &&
        c.getTime() <  w2.getTime()) {
        interlock = true;
    }
    for (int i=0; i<256; i+=BREATHING_BRIGHTNESS_OFFSET) {
        setWeatherLed(w1, w2, i);
        if (interlock) {
            setCurrentTimeLed(c, i);
        }
        pixels.show();
        delay(BREATHING_INTERVAL);
    }
}

static void showWeatherGradually(WeatherDataOne &c,
                                 WeatherDataOne &f0, WeatherDataOne &f1,
                                 WeatherDataOne &f2, WeatherDataOne &f3,
                                 WeatherDataOne &f4, WeatherDataOne &f5,
                                 WeatherDataOne &f6, WeatherDataOne &f7)
{
    breathingWeatherGradually(c, c, f0);
    delay(250);

    breathingWeatherGradually(c, f0, f1);
    delay(250);

    breathingWeatherGradually(c, f1, f2);
    delay(250);

    breathingWeatherGradually(c, f2, f3);
    delay(250);

    WeatherDataOne c12(c.getTime() + SEC_PER_HARF_DAY, f3.getWeather());
    breathingWeatherGradually(c, f3, c12);
    delay(250);

    breathingWeatherGradually(c, c12, f4);
    delay(250);

    breathingWeatherGradually(c, f4, f5);
    delay(250);

    breathingWeatherGradually(c, f5, f6);
    delay(250);

    breathingWeatherGradually(c, f6, f7);
    delay(250);

    WeatherDataOne c24(c.getTime() + SEC_PER_HARF_DAY * 2, f7.getWeather());
    breathingWeatherGradually(c, f7, c24);
    delay(250);
}

void setup()
{
    const int CAPACITY_OF_RECORDS = 32;
    const int CAPACITY_OF_EACH_RECORD = 64;
    Log::SetCapacity(CAPACITY_OF_RECORDS, CAPACITY_OF_EACH_RECORD);

    Serial.begin(115200);
    Serial.println("");

    pixels.begin();
    pixels.setBrightness(32);

    pinMode(MODE_PIN, INPUT);

    if (!Config::Initialize()) {
        Log::Error("Faild to execute SPIFFS.begin().");
        showError(-1);
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
    PRINT_FREE_RAM();
    weatherData.clear();
    int retry_times = 0;
    for (retry_times=0; retry_times<WEBAPI_RETRY_MAX; retry_times++) {
        PRINT_FREE_RAM();
        int result = weatherClient.getForecast5Weather(fnForecast5Weather);
        if (result != 200) {
            String log = "Failed to execute weatherClient.getForecast5Weather() which returns ";
            log += result;
            Log::Error(log.c_str());
            delay(1000);
            continue;
        }
        PRINT_FREE_RAM();
        result = weatherClient.getCurrentWeather(fnCurrentWeather);
        if (result != 200) {
            String log = "Failed to execute weatherClient.getCurrentWeather() which returns ";
            log += result;
            Log::Error(log.c_str());
            delay(1000);
            continue;
        }
        break;
    }
    if (retry_times >= WEBAPI_RETRY_MAX) {
        Log::Error("Faled to webapi retry");
        showError(60);
        // Perhaps memory leaks cause to fails webapi.
        // Restart, because increase free memory.
        ESP.restart();
        return;
    }
    String log = "Web api retry times is ";
    log += retry_times;
    Log::Info(log.c_str());

    PRINT_FREE_RAM();
    WeatherDataOne &c  = weatherData.getCurrentWeather();
    WeatherDataOne &f0 = weatherData.getForecastWeather(0);
    WeatherDataOne &f1 = weatherData.getForecastWeather(1);
    WeatherDataOne &f2 = weatherData.getForecastWeather(2);
    WeatherDataOne &f3 = weatherData.getForecastWeather(3);
    WeatherDataOne &f4 = weatherData.getForecastWeather(4);
    WeatherDataOne &f5 = weatherData.getForecastWeather(5);
    WeatherDataOne &f6 = weatherData.getForecastWeather(6);
    WeatherDataOne &f7 = weatherData.getForecastWeather(7);

    PRINT_FREE_RAM();
    breathingOut();
    for (int t=0; t<WEB_ACCESS_INTERVAL; t+=SHOW_INTERVAL) {
        PRINT_FREE_RAM();
        showWeatherGradually(c, f0, f1, f2, f3, f4, f5, f6, f7);
        PRINT_FREE_RAM();
        breathingOut();
        PRINT_FREE_RAM();
        showWeather(c, f0, f1, f2, f3);
        PRINT_FREE_RAM();
        delay(SHOW_INTERVAL);
        PRINT_FREE_RAM();
        breathingOut();
    }
    PRINT_FREE_RAM();
}
