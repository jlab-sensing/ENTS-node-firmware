#include "configuration.h"

Configuration config;

void printConfiguration() {
  Serial.println("\n=== Configuration ===");
  
  Serial.println("Upload Settings:");
  Serial.printf("  Logger ID: %u\n", config.logger_id);
  Serial.printf("  Cell ID: %u\n", config.cell_id);
  Serial.printf("  Upload Method: %s\n", config.upload_method.c_str());
  Serial.printf("  Upload Interval: %u seconds\n", config.upload_interval);
  
  Serial.println("\nMeasurement Settings:");
  Serial.printf("  Voltage Enabled: %s\n", config.voltage_enabled ? "Yes" : "No");
  Serial.printf("  Current Enabled: %s\n", config.current_enabled ? "Yes" : "No");
  Serial.printf("  Teros12 Enabled: %s\n", config.teros12_enabled ? "Yes" : "No");
  Serial.printf("  Teros21 Enabled: %s\n", config.teros21_enabled ? "Yes" : "No");
  Serial.printf("  BME280 Enabled: %s\n", config.bme280_enabled ? "Yes" : "No");
  Serial.printf("  Calibration V Slope: %.4f\n", config.calibration_v_slope);
  Serial.printf("  Calibration V Offset: %.4f\n", config.calibration_v_offset);
  Serial.printf("  Calibration I Slope: %.4f\n", config.calibration_i_slope);
  Serial.printf("  Calibration I Offset: %.4f\n", config.calibration_i_offset);
  
  if (config.upload_method == "WiFi") {
    Serial.println("\nWiFi Settings:");
    Serial.printf("  WiFi SSID: %s\n", config.wifi_ssid.c_str());
    Serial.printf("  WiFi Password: %s\n", config.wifi_password.c_str());
    Serial.printf("  API Endpoint URL: %s\n", config.api_endpoint_url.c_str());
  }
  
  Serial.println("====================\n");
}