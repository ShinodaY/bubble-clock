# bubble-clock
The time is displayed in liquid by glowing air-bubbles. You can see on Youtube and instructables.
https://youtu.be/hw06hDYq7kQ
https://www.instructables.com/id/Glowing-Air-Bubble-Clock-Powerd-by-ESP8266

8 solenoid valves eject air into glycerin roughly separated with walls. WiFi connected ESP8266 controls eight solenoid valves via I/O expander; I2C Interface, so that to display correct time on air bubbles and small OLED display. 8 NeoPixel leds illuminate bubbles.

Please refer to other article regarding to ESP8266 arduino coding and OTA uploading.
Sorry for not-smart code and Japanese comments.

Your wifi_ssid and wifi_password need to be input in line:
wifiMulti.addAP("your_wifi_ssid", "your_wifi_password");
