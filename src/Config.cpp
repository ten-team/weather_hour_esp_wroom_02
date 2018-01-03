#include "Config.h"

#include <FS.h>

const char* config_file = "/config.txt";

bool Config::Initialize()
{
    return SPIFFS.begin();
}

bool Config::ReadWifiConfig(String& ssid, String& pass)
{
    File fd = SPIFFS.open(config_file, "r");
    if (!fd) {
        return false;
    }
    ssid = fd.readStringUntil('\n');
    ssid.trim();
    pass = fd.readStringUntil('\n');
    pass.trim();
    fd.close();
    return true;
}

bool Config::WriteWifiConfig(const String& ssid, const String& pass)
{
    File fd = SPIFFS.open(config_file, "w");
    if (!fd) {
        return false;
    }
    fd.println(ssid);
    fd.println(pass);
    fd.close();
    return true;
}
