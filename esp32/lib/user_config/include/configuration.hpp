#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// AP Configuration
constexpr const char* AP_SSID = "ESP32-C3-Config";
constexpr const char* AP_PASSWORD = "configureme";

// Configuration structure
typedef struct {
  // Upload Settings
  uint32_t logger_id;
  uint32_t cell_id;
  String upload_method;
  uint32_t upload_interval;
  
  // Measurement Settings
  bool voltage_enabled;
  bool current_enabled;
  bool teros12_enabled;
  bool teros21_enabled;
  bool bme280_enabled;
  double calibration_v_slope;
  double calibration_v_offset;
  double calibration_i_slope;
  double calibration_i_offset;
  
  // WiFi Settings
  String wifi_ssid;
  String wifi_password;
  String api_endpoint_url;
  uint32_t api_endpoint_port;
  
} Configuration;

extern Configuration config;

void printConfiguration();

#endif