#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "Log.h"
#include "Config.h"
#include "ConfigServer.h"
#include "NCMBConfig.h"
#include "YudetamagoClient.h"

#define MODE_PIN          16
#define NEO_PIXEL_PIN     5
#define NUM_OF_NEO_PIXELS 32
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
const int NCMB_ACCESS_INTERVAL  = (5 * 60 * 1000);

static void showError() {
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

static void reconnectWifi() {
    String ssid;
    String pass;
    if (!Config::ReadWifiConfig(ssid, pass)) {
        Log::Error("Faild to read config.");
        showError();
    }
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

static void showExistState() {
}

void setup() {
    Serial.begin(115200);
    Serial.println("");

    pixels.begin();
    pixels.setBrightness(255);

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
    showExistState();
}

void loop() {
    showExistState();
    delay(NCMB_ACCESS_INTERVAL);
}
