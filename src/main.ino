#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>

#include "Config.h"
#include "ConfigServer.h"

#define MODE_PIN            16
#define LED_PIN             13
#define NEO_PIXEL_PIN       5
#define NUM_OF_NEO_PIXELS   1
#define NEO_PIXEL_LED_INDEX 0

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_OF_NEO_PIXELS,
                                             NEO_PIXEL_PIN,
                                             NEO_GRB + NEO_KHZ800);

const uint32_t BLACK_COLOR   = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t ERROR_COLOR   = Adafruit_NeoPixel::Color(255, 0, 0);
const uint32_t WAITING_COLOR = Adafruit_NeoPixel::Color(255, 255, 51);
const uint32_t CONFIG_COLOR  = Adafruit_NeoPixel::Color(255, 255, 255);

static void showError() {
    while (true) {
        pixels.setPixelColor(NEO_PIXEL_LED_INDEX, ERROR_COLOR);
        pixels.show();
        delay(1000);

        pixels.setPixelColor(NEO_PIXEL_LED_INDEX, BLACK_COLOR);
        pixels.show();
        delay(1000);
    }
}

static void reconnectWifi() {
    String ssid;
    String pass;
    if (!Config::ReadWifiConfig(ssid, pass)) {
        Serial.println("Faild to read config.");
        showError();
    }
    // If forget mode(WIFI_STA), mode might be WIFI_AP_STA.
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    while (WiFi.status() != WL_CONNECTED) {
        pixels.setPixelColor(NEO_PIXEL_LED_INDEX, WAITING_COLOR);
        pixels.show();
        Serial.print(".");
        delay(500);

        pixels.setPixelColor(NEO_PIXEL_LED_INDEX, BLACK_COLOR);
        pixels.show();
        Serial.print(".");
        delay(500);
    }

    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);
    Serial.println("");

    pixels.begin();
    pixels.setBrightness(255);

    pinMode(LED_PIN,  OUTPUT);
    pinMode(MODE_PIN, INPUT);

    if (!Config::Initialize()) {
        Serial.println("Faild to execute SPIFFS.begin().");
        showError();
    }

    if (digitalRead(MODE_PIN) == LOW) {
        Serial.println("Detected Config mode.");
        pixels.setPixelColor(NEO_PIXEL_LED_INDEX, CONFIG_COLOR);
        pixels.show();
        ConfigServer::Start();
        // can not reach here.
    }
    Serial.println("Detected Normal mode.");
    reconnectWifi();
}

void loop() {
    pixels.setPixelColor(NEO_PIXEL_LED_INDEX, 255, 0, 0);
    pixels.show();
    digitalWrite(LED_PIN, HIGH);
    delay(1000);

    pixels.setPixelColor(NEO_PIXEL_LED_INDEX, 0, 255, 0);
    pixels.show();
    digitalWrite(LED_PIN, LOW);
    delay(1000);
}
