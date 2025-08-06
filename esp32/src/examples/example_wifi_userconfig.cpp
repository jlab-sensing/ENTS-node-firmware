#include <WiFi.h>

#include "config_server.hpp"

const std::string AP_SSID = "ents";
const std::string AP_PASSWORD = "changeme";

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Setup server
  setupServer();
  Serial.println("HTTP server started");
}

void loop() { handleClient(); }
