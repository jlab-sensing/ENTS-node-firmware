/**
 * @brief Main file for the Soil Power Sensor firmware
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-28
 */

 #include <Arduino.h>
 #include <ArduinoLog.h>
 #include <Wire.h>
 #include <WiFi.h>
 #include <sys/time.h>  // For time functions
 
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
 
 // Create irrigation module
 static ModuleIrrigation irrigation;
 
 // External declarations for time initialization flags (defined in irrigation.cpp)
bool time_initialized;
bool time_sync_successful;
 
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
 
   // Create logging interface
   Log.begin(LOG_LEVEL_TRACE, &Serial);
   Log.noticeln("ESP32 Starting...");
 
   // Connect to WiFi
   Log.noticeln("Connecting to WiFi: %s", ssid.c_str());
   WiFi.begin(ssid.c_str(), password.c_str());
 
   int wifi_retries = 0;
   const int max_wifi_retries = 20; // 10 seconds total
   
   while (WiFi.status() != WL_CONNECTED && wifi_retries < max_wifi_retries) {
     delay(500);
     Serial.print(".");
     wifi_retries++;
   }
 
   if (WiFi.status() != WL_CONNECTED) {
     Log.errorln("WiFi connection failed!");
     // Continue anyway - time sync will fail but other functions should work
   } else {
     Log.noticeln("");
     Log.noticeln("WiFi connected");
     Log.noticeln("IP address: %s", WiFi.localIP().toString().c_str());
     
     // Configure NTP time synchronization after WiFi is connected
     Log.noticeln("Configuring NTP time...");
     configTzTime("UTC", "pool.ntp.org", "time.nist.gov", "time.google.com");
     
     // Wait a moment for time to sync
     delay(2000);
     
     // Check if time is synchronized
     time_t now = time(nullptr);
     if (now > 1000000000) {
       Log.noticeln("Time synchronized: %s", ctime(&now));
       time_sync_successful = true;
     } else {
       Log.errorln("Time not synchronized, using system time");
       time_sync_successful = false;
     }
     time_initialized = true;
   }
 
   // Setup Web server (doesn't need time sync)
   SetupServer();
   Log.noticeln("Web server started");
 
   Log.noticeln("ents-node esp32 firmware, compiled at %s %s", __DATE__,
                __TIME__);
   Log.noticeln("Git SHA: %s", GIT_REV);
 
   Log.noticeln("Starting i2c interface...");
 
   //Register irrigation module
   mh.RegisterModule(&irrigation);
 
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
   irrigation.CheckAutoIrrigation();
   delay(20);
 }
