#include <Arduino.h>

int unixtimeToHour(time_t t);

class WeatherClient
{
public:
    WeatherClient() {}
    ~WeatherClient() {}

public:
    void SetLongitudeAndLatitude(const String &lat, const String &lon);

    /**
     * @see https://openweathermap.org/current
     */
    int GetCurrentWeather(void (*fn)(time_t t, const char *main));

    /**
     * @see https://openweathermap.org/forecast5
     */
    int GetForecast5Weather(void (*fn)(time_t t, const char *main));

private:
    String lat;
    String lon;
};
