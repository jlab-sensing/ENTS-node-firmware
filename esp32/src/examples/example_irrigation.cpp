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

  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup webserver
  SetupServer();
}

void loop() {
  HandleClient();
  delay(20);
}
