/**
 * @brief Main file for the Soil Power Sensor firmware
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-28
 */

#include <Arduino.h>
#include <WiFi.h>

#include "dirtviz.hpp"

/** Baud rate for serial interface */
#define SERIAL_BAUD 115200

const char ssid[] = "HARE_Lab";
const char pass[] = "";

Dirtviz api("dirtviz.jlab.ucsc.edu", 443);

const char data[] = "Hello World";
const size_t data_len = 12;

/**
 * @brief Initialization code run on startup
 *
 *
 */
void setup() {
  // Start serial interface
  Serial.begin(115200);
  // Wait for serial connection
  while (!Serial) {
    delay(100);
  }

  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(pass);
  Serial.print("Connecting");

  // Connect to WiFi network
  WiFi.begin(ssid, pass);

  // Wait for WiFi to connect
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("Connected!");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Buffer to store response
  static uint8_t resp[256];
  static size_t resp_len;

  // Send example measurement
  resp_len = api.SendMeasurement((const uint8_t*)data, data_len);

  // Print response
  Serial.print("Response:");
  Serial.println(reinterpret_cast<char*>(resp));
}
