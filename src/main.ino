#include <Adafruit_NeoPixel.h>
#include "Config.h"
#include "ConfigServer.h"

#define MODE_PIN          16
#define LED_PIN           13
#define NEO_PIXEL_PIN     5
#define NUM_OF_NEO_PIXELS 1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_OF_NEO_PIXELS,
                                             NEO_PIXEL_PIN,
                                             NEO_GRB + NEO_KHZ800);

void setup() {
    Serial.begin(115200);

    pixels.begin();
    pixels.setBrightness(255);

    pinMode(LED_PIN,  OUTPUT);
    pinMode(MODE_PIN, INPUT);

    Config::Initialize();

    if (digitalRead(MODE_PIN) == LOW) {
        Serial.println("Detected Config mode.");
        ConfigServer::Start();
        // can not reach here.
    } else {
        Serial.println("Detected Normal mode.");
    }

}

void loop() {
    pixels.setPixelColor(0, 255, 0, 0);
    pixels.show();
    digitalWrite(LED_PIN, HIGH);
    delay(1000);

    pixels.setPixelColor(0, 0, 255, 0);
    pixels.show();
    digitalWrite(LED_PIN, LOW);
    delay(1000);
}
