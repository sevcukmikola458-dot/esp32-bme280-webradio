/*
  ESP32: Метеостанція BME280 + Вебрадіо (інтернет-радіо, 5 станцій) + Веб-інтерфейс

  Підключення:
    BME280      -> SDA=21, SCL=22, VCC=3.3V, GND=GND (адреса 0x76)
    MAX98357A   -> BCLK=27, LRC=26, DIN=25, VIN=5V, GND=GND, + динамік
    Кнопка      -> GPIO4 -> GND (INPUT_PULLUP), перемикає станції по колу

  Бібліотеки (Library Manager або GitHub):
    - Adafruit BME280 Library
    - Adafruit Unified Sensor
    - ESP32-audioI2S (автор: schreibfaul1)
      https://github.com/schreibfaul1/ESP32-audioI2S

  Після прошивки відкрийте в Serial Monitor IP-адресу пристрою
  і введіть її в браузері телефону (в тій же Wi-Fi мережі) -
  побачите показники датчика і кнопки перемикання станцій.

  ВАЖЛИВО: впишіть свої дані Wi-Fi нижче (WIFI_SSID / WIFI_PASSWORD).
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include
