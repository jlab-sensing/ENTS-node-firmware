#include "protobuf_utils.h"

#include <Arduino.h>

void printEncodedData(const uint8_t *data, size_t len) {
  Serial.println("\n=== Encoded Protobuf Data ===");
  Serial.print("Length: ");
  Serial.println(len);
  Serial.print("Data: ");
  for (size_t i = 0; i < len; i++) {
    if (i > 0) Serial.print(" ");
    Serial.printf("%02X", data[i]);
  }
  Serial.println("\n============================");
}

void printDecodedConfig(const UserConfiguration *pb_config) {
  Serial.println("\n=== Decoded Configuration ===");

  Serial.println("Upload Settings:");
  Serial.printf("  Logger ID: %u\n", pb_config->logger_id);
  Serial.printf("  Cell ID: %u\n", pb_config->cell_id);
  Serial.printf(
      "  Upload Method: %s\n",
      pb_config->Upload_method == Uploadmethod_WiFi ? "WiFi" : "LoRa");
  Serial.printf("  Upload Interval: %u seconds\n", pb_config->Upload_interval);

  Serial.println("\nMeasurement Settings:");
  Serial.print("  Enabled Sensors: ");
  for (size_t i = 0; i < pb_config->enabled_sensors_count; i++) {
    if (i > 0) Serial.print(", ");
    switch (pb_config->enabled_sensors[i]) {
      case EnabledSensor_Voltage:
        Serial.print("Voltage");
        break;
      case EnabledSensor_Current:
        Serial.print("Current");
        break;
      case EnabledSensor_Teros12:
        Serial.print("Teros12");
        break;
      case EnabledSensor_Teros21:
        Serial.print("Teros21");
        break;
      case EnabledSensor_BME280:
        Serial.print("BME280");
        break;
    }
  }
  Serial.println();
  Serial.printf("  Voltage Slope: %.4f\n", pb_config->Voltage_Slope);
  Serial.printf("  Voltage Offset: %.4f\n", pb_config->Voltage_Offset);
  Serial.printf("  Current Slope: %.4f\n", pb_config->Current_Slope);
  Serial.printf("  Current Offset: %.4f\n", pb_config->Current_Offset);

  if (pb_config->Upload_method == Uploadmethod_WiFi) {
    Serial.println("\nWiFi Settings:");
    Serial.printf("  WiFi SSID: %s\n", pb_config->WiFi_SSID);
    Serial.printf("  WiFi Password: %s\n", pb_config->WiFi_Password);
    Serial.printf("  API Endpoint URL: %s\n", pb_config->API_Endpoint_URL);
  }

  Serial.println("============================\n");
}