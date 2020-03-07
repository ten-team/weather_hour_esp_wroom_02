# weather_hour_esp_wroom_02

## for user

### How to set up

1. [weatherhour]  Power on with button x
1. [Smart phone] Access wifi
   SSID : weatherhour_config
   PASS : weatherhour
1. [Smart phone] Access using web browser
   http://192.168.4.1
1. [Smart phone] Config your SSID, and PASSWORD
1. [weatherhour]  Power OFF, and Power ON

## for debug

### build/upload

```
$ pio run --target upload
```

### monitor

```
$ pio device monitor | ts "%H:%M:%S" | tee log.txt
```

### build/upload/monitor

```
$ pio run --target upload && pio device monitor | ts "%H:%M:%S" | tee log.txt
```

## Library

- Adafruit NeoPixel Library (LGPL v3.0)
  https://github.com/adafruit/Adafruit_NeoPixel
- Cryptographic suite for Arduino (SHA, HMAC-SHA) 
  https://github.com/simonratner/Arduino-SHA-256
  (Fix compile errors)

## ToDo

- Error log(optional)
  - View from /admin.html
- 5 button(optional)
