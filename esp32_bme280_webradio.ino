/*
  ESP32: Метеостанція BME280 + Вебрадіо (інтернет-радіо, 5 станцій)

  Підключення:
    BME280      -> SDA=21, SCL=22, VCC=3.3V, GND=GND (адреса 0x76)
    MAX98357A   -> BCLK=27, LRC=26, DIN=25, VIN=5V, GND=GND, + динамік
    Кнопка      -> GPIO4 -> GND (INPUT_PULLUP), перемикає станції по колу

  Бібліотеки (Library Manager або GitHub):
    - Adafruit BME280 Library
    - Adafruit Unified Sensor
    - ESP32-audioI2S (автор: schreibfaul1)
      https://github.com/schreibfaul1/ESP32-audioI2S

  ВАЖЛИВО: впишіть свої дані Wi-Fi нижче (WIFI_SSID / WIFI_PASSWORD).
*/

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "Audio.h"

// ---- Wi-Fi ----
const char* WIFI_SSID     = "ASUS";
const char* WIFI_PASSWORD = "1135432906";

// ---- I2S піни (MAX98357A) ----
#define I2S_BCLK 27
#define I2S_LRC  26
#define I2S_DOUT 25

// ---- I2C піни ----
#define SDA_PIN 21
#define SCL_PIN 22
#define BME_ADDR 0x76

// ---- Кнопка ----
#define BUTTON_PIN 4

Adafruit_BME280 bme;
Audio audio;

// 5 станцій вебрадіо (приклади - замініть на власні URL потокових MP3/AAC станцій)
const char* stations[5] = {
  "http://ice1.somafm.com/groovesalad-128-mp3",
  "http://streams.radiobob.de/bob-national/mp3-192/mediaplayer",
  "http://icecast.omroep.nl/radio1-bb-mp3",
  "http://uir.stream.laut.fm/uir",
  "http://stream.dancewave.online/dance.mp3"
};
const char* stationNames[5] = {
  "SomaFM Groove Salad",
  "Radio Bob",
  "NPO Radio 1",
  "Ukrainian Internet Radio",
  "Dancewave"
};

int currentStation = 0;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 30000; // раз на 30с (щоб не заважати аудіо-стріму)

bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void setup() {
  Serial.begin(115200);
  delay(200);

  // --- I2C / BME280 ---
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bme.begin(BME_ADDR, &Wire)) {
    Serial.println("BME280 не знайдено! Перевірте адресу (0x76 або 0x77).");
  } else {
    Serial.println("BME280 ініціалізовано.");
  }

  // --- Wi-Fi ---
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Підключення до Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Підключено, IP: ");
  Serial.println(WiFi.localIP());

  // --- Аудіо ---
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12); // 0-21

  playStation(currentStation);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("Готово. Кнопка на GPIO4 перемикає станції.");
}

void loop() {
  audio.loop(); // обов'язково викликати часто - обробляє потік

  handleButton();

  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();
    printSensorData();
  }
}

void playStation(int index) {
  currentStation = index;
  Serial.print("Відтворення: ");
  Serial.println(stationNames[currentStation]);
  audio.connecttohost(stations[currentStation]);
}

void handleButton() {
  bool reading = digitalRead(BUTTON_PIN);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading == LOW && lastButtonState == HIGH) {
      int next = (currentStation + 1) % 5;
      playStation(next);
    }
  }

  lastButtonState = reading;
}

void printSensorData() {
  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float pres = bme.readPressure() / 100.0F;

  Serial.print("Темп: ");
  Serial.print(temp, 1);
  Serial.print(" C  Вологість: ");
  Serial.print(hum, 1);
  Serial.print(" %  Тиск: ");
  Serial.print(pres, 1);
  Serial.println(" гПа");
}

// --- Опційні callback-и бібліотеки ESP32-audioI2S (для діагностики) ---
void audio_info(const char *info) {
  Serial.print("audio_info: ");
  Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station: ");
  Serial.println(info);
}
