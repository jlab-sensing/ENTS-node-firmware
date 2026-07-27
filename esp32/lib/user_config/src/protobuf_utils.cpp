#include "protobuf_utils.hpp"

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
  Serial.printf("  Upload Method: %s\n",
                Uploadmethod_name(pb_config->Upload_method));
  Serial.printf("  Upload Interval: %u seconds\n", pb_config->Upload_interval);

  Serial.println("\nMeasurement Settings:");
  Serial.print("  Enabled Sensors: ");

  Serial.printf("  Enabled Sensors Multiple (%d):",
                pb_config->enabled_sensors_multiple_count);
  for (int i = 0; i < pb_config->enabled_sensors_multiple_count; i++) {
    Serial.printf(" %03d - %s\r\n", i,
                  EnabledSensor_name(
                      pb_config->enabled_sensors_multiple[i].enabled_sensor));
    Serial.printf(" %03d - cell_id=%d\r\n", i,
                  pb_config->enabled_sensors_multiple[i].cell_id);
    Serial.printf(" %03d - index=%d\r\n", i,
                  pb_config->enabled_sensors_multiple[i].index);
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
