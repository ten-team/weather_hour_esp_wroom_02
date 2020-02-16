#include <Arduino.h>

int unixtimeToJstHour(time_t t);
int unixtimeToHour(time_t t);
int unixtimeToMinute(time_t t);

class WeatherClient
{
public:
    WeatherClient() {}
    ~WeatherClient() {}

public:
    void setLongitudeAndLatitude(const String &lat, const String &lon);

    /**
     * @see https://openweathermap.org/current
     */
    int getCurrentWeather(void (*fn)(time_t t, const char *main));

    /**
     * @see https://openweathermap.org/forecast5
     */
    int getForecast5Weather(void (*fn)(int index, time_t t, const char *main));

private:
    String lat;
    String lon;
};
