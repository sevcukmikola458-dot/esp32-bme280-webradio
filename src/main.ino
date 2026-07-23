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
*/

#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Audio.h>

const char* WIFI_SSID     = "ASUS";
const char* WIFI_PASSWORD = "1135432906";

#define I2S_BCLK 27
#define I2S_LRC  26
#define I2S_DOUT 25

#define SDA_PIN 21
#define SCL_PIN 22
#define BME_ADDR 0x76

#define BUTTON_PIN 4

Adafruit_BME280 bme;
Audio audio;
WebServer server(80);

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

float lastTemp = 0, lastHum = 0, lastPres = 0;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 30000;

bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;

void readSensor();
void playStation(int index);
void handleButton();
void handleRoot();
void handleStation();

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bme.begin(BME_ADDR, &Wire)) {
    Serial.println("BME280 not found");
  } else {
    Serial.println("BME280 OK");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected! Open in browser: http://");
  Serial.println(WiFi.localIP());

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12);

  readSensor();
  playStation(currentStation);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  server.on("/", handleRoot);
  server.on("/station", handleStation);
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  audio.loop();
  server.handleClient();
  handleButton();

  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();
    readSensor();
  }
}

void readSensor() {
  lastTemp = bme.readTemperature();
  lastHum = bme.readHumidity();
  lastPres = bme.readPressure() / 100.0F;

  Serial.print("Temp: ");
  Serial.print(lastTemp, 1);
  Serial.print(" C  Hum: ");
  Serial.print(lastHum, 1);
  Serial.print(" %  Pres: ");
  Serial.print(lastPres, 1);
  Serial.println(" hPa");
}

void playStation(int index) {
  currentStation = index;
  Serial.print("Playing: ");
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

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>ESP32 Weather + Web Radio</title>";
  html += "<style>";
  html += "body{font-family:sans-serif;background:#1a1a1a;color:#eee;text-align:center;padding:20px;}";
  html += "h1{font-size:20px;} .sensor{font-size:18px;margin:20px 0;}";
  html += ".btn{display:block;width:90%;margin:8px auto;padding:14px;font-size:16px;";
  html += "border-radius:8px;border:none;background:#333;color:#eee;}";
  html += ".active{background:#2a7a2a;font-weight:bold;}";
  html += "</style></head><body>";
  html += "<h1>Weather Station + Web Radio</h1>";
  html += "<div class='sensor'>Temp: " + String(lastTemp, 1) + " &deg;C<br>";
  html += "Humidity: " + String(lastHum, 1) + " %<br>";
  html += "Pressure: " + String(lastPres, 1) + " hPa</div>";
  html += "<div>";
  for (int i = 0; i < 5; i++) {
    html += "<a href='/station?i=" + String(i) + "'>";
    html += "<button class='btn" + String(i == currentStation ? " active" : "") + "'>";
    html += String(stationNames[i]);
    html += "</button></a>";
  }
  html += "</div></body></html>";
  server.send(200, "text/html; charset=utf-8", html);
}

void handleStation() {
  if (server.hasArg("i")) {
    int idx = server.arg("i").toInt();
    if (idx >= 0 && idx < 5) {
      playStation(idx);
    }
  }
  server.sendHeader("Location", "/");
  server.send(303);
}
