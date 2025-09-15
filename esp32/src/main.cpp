/**
 * @brief Main file for the Soil Power Sensor firmware
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-28
 */

#include <Arduino.h>
#include <ArduinoLog.h>
#include <Wire.h>

#include "module_handler.hpp"
#include "modules/irrigation.hpp"
#include "modules/microsd.hpp"
#include "modules/wifi.hpp"

/** Target device address */
static const uint8_t dev_addr = 0x20;
/** Serial data pin */
static const int sda_pin = 0;
/** Serial clock pin */
static const int scl_pin = 1;

const std::string ssid = "HARE_Lab";
const std::string password = "";

// create wifi module
static ModuleHandler::ModuleHandler mh;
  
// create and register the microSD module
 static ModuleMicroSD microSD;
  
// create and register the WiFi module
static ModuleWiFi wifi;

// create and register the irrigation module
//static ModuleIrrigation irrigation;

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
  Log.begin(LOG_LEVEL_TRACE, &Serial);

  // Needed for irrigation
  //WiFi.begin(ssid.c_str(), password.c_str());

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Setup Web server
  SetupServer();

  Log.noticeln("Web server started");
  // Log.noticeln("Connect to ESP32 AP and visit http://%s",
  // WiFi.softAPIP().toString().c_str());

  Log.noticeln("ents-node esp32 firmware, compiled at %s %s", __DATE__,
               __TIME__);
  Log.noticeln("Git SHA: %s", GIT_REV);

  Log.noticeln("Starting i2c interface...");

  //mh.RegisterModule(&irrigation);

  mh.RegisterModule(&wifi);

  mh.RegisterModule(&microSD);

  // start i2c interface
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  bool i2c_status = Wire.begin(dev_addr, sda_pin, scl_pin, 100000);

  if (i2c_status) {
    Log.noticeln("Success!");
  } else {
    Log.noticeln("Failed!");
  }
}

/** Loop code */
void loop() {
  HandleClient();
  delay(20);
}
