/**
 * @brief Main file for the Soil Power Sensor firmware
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-28
 */

#include <Arduino.h>
#include <ArduinoLog.h>
#include <WiFi.h>
#include <Wire.h>
#include <LittleFS.h>

#include "config_server.hpp"
#include "module_handler.hpp"
#include "modules/wifi.hpp"
#include "modules/wifi_userconfig.hpp"

/** Target device address */
static const uint8_t dev_addr = 0x20;
/** Serial data pin */
static const int sda_pin = 0;
/** Serial clock pin */
static const int scl_pin = 1;
  
// create module handler
static ModuleHandler::ModuleHandler mh;

// NOTE these variables must be relevant for the entire program lifetime
// create wifi module
static ModuleWiFi wifi;
// create user config module
static ModuleHandler::ModuleUserConfig user_config;

/**
 * @brief Callback for onReceive
 *
 * See Arduino wire library for reference
 */
void onReceive(int len) {
  Log.traceln("onReceive(%d)", len);
  mh.OnReceive(len);
}

/**
 * @brief Callback for onRequest
 *
 * See Arduino wire library for reference
 */
void onRequest() {
  Log.traceln("onRequest");
  mh.OnRequest();
}

/** Startup code */
void setup() {
  // Start serial interface
  Serial.begin(115200);

  // Create logging interfface
  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  Log.verbose(R"(
-------------------------------------------------------------------------------

RESET!

-------------------------------------------------------------------------------
)");

  Log.noticeln("ents-node esp32 firmware, compiled at %s %s", __DATE__,
               __TIME__);
  Log.noticeln("Git SHA: %s", GIT_REV);

  // Start I2C interface
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  bool i2c_status = Wire.begin(dev_addr, sda_pin, scl_pin, 100000);
  if (!i2c_status) {
    Log.errorln("I2C initialization failed!");
    while (1);
  }

  // initialize filesystem
  if (!LittleFS.begin()) {
    Log.errorln("LittleFS mount failed!");
    while (1);
  }
  
  //WiFi.mode(WIFI_AP);
  //WiFi.softAP(AP_SSID.c_str(), AP_PASSWORD.c_str());
  //Log.noticeln("Access Point started");
  //Log.noticeln("IP Address: %s", WiFi.softAPIP().toString().c_str());


  // Register modules
  mh.RegisterModule(&wifi);
  mh.RegisterModule(&user_config);
}

void loop() {
  // Handle web server requests
  handleClient();

  // Other main loop tasks can go here
  delay(10);
}
