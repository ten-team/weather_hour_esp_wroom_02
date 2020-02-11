#include <Arduino.h>

class Config
{
public:
    static bool Initialize();
    static bool ReadWifiConfig(String& ssid, String& pass, String& lat, String& lon);
    static bool WriteWifiConfig(const String& ssid, const String& pass, const String& lat, const String& lon);
};

