#include "YudetamagoClient.h"

#include <ESP8266HTTPClient.h>
#include "NCMBConfig.h"

bool YudetamagoClient::GetExistance(const char* objectId)
{
    // You should specify NCMB finger print(SHA1)
    // 1. Access https://mb.api.cloud.nifty.com/ from Browser
    // 2. Confirm server certification
    //
    // see also
    // https://github.com/esp8266/Arduino/issues/1941
    // https://github.com/esp8266/Arduino/issues/2556
    const char *fingerprint = "ED:74:40:90:98:20:C4:0A:C3:DD:9B:66:8C:51:8F:DF:0F:EC:53:11";

    String url = "https://mb.api.cloud.nifty.com/2013-09-01/classes/ToggleStocker?where=%7B%22objectId%22%3A%22";
    url += objectId;
    url += "%22%7D";
    HTTPClient http;
    http.begin(url, fingerprint);
    http.addHeader("X-NCMB-Application-Key", APPLICATION_KEY);
    http.addHeader("X-NCMB-Timestamp",       TIMESTAMP);
    http.addHeader("X-NCMB-Signature",       SIGNATURE);
    http.addHeader("Content-Type",           "application/json");
    int httpCode = http.GET();
    if (httpCode < 0) {
        Serial.print("http GET error: ");
        Serial.print(httpCode, DEC);
        Serial.println(": " + http.errorToString(httpCode));
        http.end();
        return false;
    }

    Serial.print("http GET returns ");
    Serial.print(httpCode, DEC);
    Serial.println("");
    Serial.println(http.getString());
    http.end();
    return true;
}

