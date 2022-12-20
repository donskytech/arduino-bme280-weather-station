/*
  Title:  Arduino BME/BMP280 Weather Station Dashboard
  Author: donsky
  For:    www.donskytech.com
  Date:   Dec 20, 2022
*/
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "AsyncJson.h"
#include "ArduinoJson.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)


/*
  Replace the SSID and Password according to your wifi
*/
const char *ssid = "<REPLACE_WITH_YOUR_WIFI_SSID>";
const char *password = "<REPLACE_WITH_YOUR_WIFI_PASSWORD";

// Webserver setup
AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

Adafruit_BME280 bme; // I2C
// Adafruit_BME280 bme(BME_CS); // hardware SPI
// Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // time to get serial running

  // Initialize Filesystem LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("Cannot load LittleFS Filesystem!");
    return;
  }

  // Connect to WIFI
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }
  String localIPAddress = WiFi.localIP().toString();
  Serial.print("IP Address: ");
  Serial.println(localIPAddress);

  // Route for root index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", String(), false); });

  // Route for root style.css
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/style.css", "text/css"); });

  // Route for root main.js
  server.on("/main.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/main.js", "text/javascript"); });

  // Respond to /sensorReadings event
  server.on("/sensorReadings", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    DynamicJsonDocument json(1024);
    json["status"] = "ok";
    json["temperature"] = bme.readTemperature();
    json["pressure"] = bme.readPressure() / 100.0F;
    json["altitude"] = bme.readAltitude(SEALEVELPRESSURE_HPA);
    json["humidity"] = bme.readHumidity();
    serializeJson(json, *response);
    request->send(response);
  });

  server.onNotFound(notFound);

  server.begin();

  unsigned status;

  // default settings
  status = bme.begin(0x76);
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status)
  {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    Serial.print("SensorID was: 0x");
    Serial.println(bme.sensorID(), 16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1)
      delay(10);
  }
}

void loop()
{
  // Nothing in here
}
