/**
 * @example example_irrigation.cpp
 *
 * This example demonstrates the webserver to control an irrigation value.
 *
 * @author John Madden
 * @date 2025-08-19
 */

#include <Arduino.h>
#include <ArduinoLog.h>
#include <WiFi.h>

#include "webserver.hpp"

const std::string ssid = "HARE_Lab";
const std::string password = "";

void setup() {
  Serial.begin(115200);
  delay(3000);  // Give time for serial to initialize
  
  Serial.println("ESP32-C3 Starting...");
  
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);
  Serial.println("Logging initialized");

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("-------------------");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //THIS IS THE ACTUAL IP
  } else {
    Serial.println("-------------------");
    Serial.println("WiFi connection FAILED!");
    Serial.print("WiFi status: ");
    Serial.println(WiFi.status());
    while(1) { delay(1000); }  // Stop here if WiFi fails
  }

  SetupServer();
  Serial.println("Web server started");
}

void loop() {
  // Add heartbeat to know it's running
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    Serial.println("Web server running...");
    lastPrint = millis();
  }
  
  HandleClient();
  delay(20);
}
