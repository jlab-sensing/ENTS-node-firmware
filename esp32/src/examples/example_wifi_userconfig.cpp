#include <WiFi.h>

#include "config_server.h"
#include "configuration.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.println("Access Point started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Setup server
  setupServer();
  Serial.println("HTTP server started");
}

void loop() { handleClient(); }