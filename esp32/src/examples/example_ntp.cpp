/**
 * @example example_ntp.cpp
 *
 * Example of using ESP32 as an NTP client. Connects to a WiFi network and
 * print the current time over serial.
 *
 * @author Varun Sreedharan <vasreedh@ucsc.edu>
 * @date 2024-09-03
 */

#include <Arduino.h>
#include <NTPClient.h>
#include <WiFi.h>

/** Baud rate for serial interface */
#define SERIAL_BAUD 115200

const char ssid[] = "Enter-Your-SSID";
const char pass[] = "Enter-Your-Password";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

String currentTime;

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

  timeClient.begin();
  // Set time offset in seconds
  // GMT = 0
  // California: PST (GMT -7 hours) = -25200
  // Georgia: EST (GMT -4 hours) = -14400
  // Illinois: CDT (GMT -5 hours) = -18000
  timeClient.setTimeOffset(-28800);
}

void loop() {
  // Prints time in HH:MM:SS format
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  currentTime = timeClient.getFormattedTime();
  Serial.print("The time is: ");
  Serial.println(currentTime);

  // Wait 1 second
  delay(1000);
}
