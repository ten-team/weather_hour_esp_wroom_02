#include <Arduino.h>

class WeatherClient
{
public:
    WeatherClient() {}
    ~WeatherClient() {}

public:
    void SetLongitudeAndLatitude(const String &lat, const String &lon);
    int GetForecast5Weather();

private:
    String lat;
    String lon;
};
