#include <Arduino.h>

class YudetamagoClient
{
public:
    static bool GetExistance(const char *objectId, bool& exists, String& error);
    static bool SetExistance(const char *objectId, bool exists, String& error);
};
