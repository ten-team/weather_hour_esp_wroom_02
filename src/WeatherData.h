#include <Arduino.h>
#include <assert.h>

class WeatherDataOne
{
public:
    WeatherDataOne() : t(0), weather() {}
    WeatherDataOne(time_t t, String weather) : t(t), weather(weather) {}
    ~WeatherDataOne() {}

public:
    time_t getTime() {
        return t;
    }

    void setTime(time_t t) {
        this->t = t;
    }

    String& getWeather() {
        return weather;
    }

    void setWeather(String weather) {
        this->weather = weather;
    }

    void clear() {
        t = 0;
        weather = "";
    }

private:
    time_t t;
    String weather;
};

class WeatherData
{
public:
    WeatherData() {}
    ~WeatherData() {}

public:
    WeatherDataOne& getCurrentWeather() {
        return current;
    }

    WeatherDataOne& getForecastWeather(int i) {
        if (i < 0 || i > FORECAST_CAPACITY) {
            assert(0 && "i < 0 || i > FORECAST_CAPACITY");
            return forecasts[0];
        }
        return forecasts[i];
    }

    void clear() {
        current.clear();
        for (int i=0; i<FORECAST_CAPACITY; i++) {
            forecasts[i].clear();
        }
    }

private:
    enum {FORECAST_CAPACITY = 8};
    WeatherDataOne current;
    WeatherDataOne forecasts[FORECAST_CAPACITY];
};
