#include <Arduino.h>

class Config
{
public:
    static bool Initialize();
    static bool ReadWifiConfig(String& ssid, String& pass);
    static bool WriteWifiConfig(const String& ssid, const String& pass);
};

