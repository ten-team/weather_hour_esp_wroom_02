#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "Log.h"
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
#define BREATHING_BRIGHTNESS_OFFSET 32
#define BREATHING_INTERVAL          160

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
const uint32_t WEATHER_COLOR_SNOW    = Adafruit_NeoPixel::Color(242, 242, 255);
const uint32_t WEATHER_COLOR_UNKNOWN = Adafruit_NeoPixel::Color(255, 0, 0);

const int WEB_ACCESS_INTERVAL   = (5 * 60 * 1000);
const int SHOW_WEATHER_TIME     = (2400);

WeatherClient weatherClient;
WeatherData   weatherData;

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

static int getDelayTimeWhenConnectingWifi(int index)
{
    int d = (NUM_OF_NEO_PIXELS / 2 - index);
    if (d < 0) {
        d = -d;
    }
    d = NUM_OF_NEO_PIXELS / 2 - d;
    return d * d;
}

static void showConnectingWifi()
{
    clearLeds();
    pixels.show();
    delay(10);

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
        showError();
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
    pixels.setPixelColor(hourIndex + minIndex, color);
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
    breathingOut();
}

static void breathingWeatherGradually(WeatherDataOne &c,
                                      WeatherDataOne &w1,
                                      WeatherDataOne &w2)
{
    for (int i=0; i<256; i+=BREATHING_BRIGHTNESS_OFFSET) {
        setWeatherLed(w1, w2, i);
        pixels.show();
        delay(BREATHING_INTERVAL);
    }
}

static void showWeatherGradually(WeatherDataOne &c,
                                 WeatherDataOne &f0, WeatherDataOne &f1,
                                 WeatherDataOne &f2, WeatherDataOne &f3)
{
    breathingWeatherGradually(c, c, f0);
    delay(500);

    breathingWeatherGradually(c, f0, f1);
    delay(500);

    breathingWeatherGradually(c, f1, f2);
    delay(500);

    breathingWeatherGradually(c, f2, f3);
    delay(500);

    WeatherDataOne c12 = c;
    c12.setTime(c.getTime() + SEC_PER_HARF_DAY);
    breathingWeatherGradually(c, f3, c12);
    breathingOut();
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
    weatherData.clear();
    int result = weatherClient.getForecast5Weather(fnForecast5Weather);
    if (result != 200) {
        String log = "Failed to execute weatherClient.getForecast5Weather() which returns ";
        log += result;
        Log::Error(log.c_str());
    }
    result = weatherClient.getCurrentWeather(fnCurrentWeather);
    if (result != 200) {
        String log = "Failed to execute weatherClient.getCurrentWeather() which returns ";
        log += result;
        Log::Error(log.c_str());
    }

    WeatherDataOne &c  = weatherData.getCurrentWeather();
    WeatherDataOne &f0 = weatherData.getForecastWeather(0);
    WeatherDataOne &f1 = weatherData.getForecastWeather(1);
    WeatherDataOne &f2 = weatherData.getForecastWeather(2);
    WeatherDataOne &f3 = weatherData.getForecastWeather(3);
    showWeatherGradually(c, f0, f1, f2, f3);
    int times = WEB_ACCESS_INTERVAL / SHOW_WEATHER_TIME;
    for (int i=0; i<times; i++) {
        showWeather(c, f0, f1, f2, f3);
    }
}
