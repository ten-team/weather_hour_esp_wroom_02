#include "Config.h"

#include <FS.h>

const char* config_file = "/config.txt";

bool Config::Initialize()
{
    return SPIFFS.begin();
}

bool Config::ReadWifiConfig(String& ssid, String& pass, String& lat, String& lon)
{
    File fd = SPIFFS.open(config_file, "r");
    if (!fd) {
        return false;
    }
    ssid = fd.readStringUntil('\n');
    ssid.trim();
    pass = fd.readStringUntil('\n');
    pass.trim();
    lat  = fd.readStringUntil('\n');
    lat.trim();
    lon  = fd.readStringUntil('\n');
    lon.trim();
    fd.close();
    return true;
}

bool Config::WriteWifiConfig(const String& ssid, const String& pass, const String& lat, const String& lon)
{
    File fd = SPIFFS.open(config_file, "w");
    if (!fd) {
        return false;
    }
    fd.println(ssid);
    fd.println(pass);
    fd.println(lat);
    fd.println(lon);
    fd.close();
    return true;
}
