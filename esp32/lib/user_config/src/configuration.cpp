#include "configuration.hpp"

#include <ArduinoLog.h>
#include <string.h>

#include "transcoder.h"

/** Instance of user config served by the webserver */
static UserConfiguration config = UserConfiguration_init_default;

void setConfig(const UserConfiguration &new_config) {
  // deeeeep copy
  config.logger_id = new_config.logger_id;
  config.cell_id = new_config.cell_id;
  config.Upload_method = new_config.Upload_method;
  config.Upload_interval = new_config.Upload_interval;
  config.enabled_sensors_count = new_config.enabled_sensors_count;
  memcpy(config.enabled_sensors, new_config.enabled_sensors,
         sizeof(config.enabled_sensors));
  config.Voltage_Slope = new_config.Voltage_Slope;
  config.Voltage_Offset = new_config.Voltage_Offset;
  config.Current_Slope = new_config.Current_Slope;
  config.Current_Offset = new_config.Current_Offset;
  strncpy(config.WiFi_SSID, new_config.WiFi_SSID, sizeof(config.WiFi_SSID));
  strncpy(config.WiFi_Password, new_config.WiFi_Password,
          sizeof(config.WiFi_Password));
  strncpy(config.API_Endpoint_URL, new_config.API_Endpoint_URL,
          sizeof(config.API_Endpoint_URL));
  config.API_Endpoint_Port = new_config.API_Endpoint_Port;

  // Check that the user config can be encoded/decoded?? (John)
  uint8_t buffer[UserConfiguration_size];
  size_t message_length = EncodeUserConfiguration(&config, buffer);
  UserConfiguration decoded_config = UserConfiguration_init_zero;

  if (message_length != -1) {
    // printEncodedData(buffer, message_length);

    if (DecodeUserConfiguration(buffer, message_length, &decoded_config) == 0) {
      // printDecodedConfig(&decoded_config);
    } else {
      Log.errorln("Failed to decode the configuration");
    }
  } else {
    Log.errorln("Failed to encode the configuration");
  }
}

const UserConfiguration &getConfig() { return config; }

String getConfigJson() {
  const UserConfiguration &config = getConfig();

  String json = "{";
  json += "\"logger_id\":" + String(config.logger_id) + ",";
  json += "\"cell_id\":" + String(config.cell_id) + ",";
  json += "\"upload_method\":\"" +
          String(config.Upload_method == Uploadmethod_WiFi ? "WiFi" : "LoRa") +
          "\",";
  json += "\"upload_interval\":" + String(config.Upload_interval) + ",";

  // Enabled sensors
  json += "\"enabled_sensors\":[";
  for (uint8_t i = 0; i < config.enabled_sensors_count; i++) {
    if (i > 0) json += ",";
    switch (config.enabled_sensors[i]) {
      case EnabledSensor_Voltage:
        json += "\"Voltage\"";
        break;
      case EnabledSensor_Current:
        json += "\"Current\"";
        break;
      case EnabledSensor_Teros12:
        json += "\"Teros12\"";
        break;
      case EnabledSensor_Teros21:
        json += "\"Teros21\"";
        break;
      case EnabledSensor_BME280:
        json += "\"BME280\"";
        break;
      default:
        break;
    }
  }
  json += "],";

  json += "\"calibration_v_slope\":" + String(config.Voltage_Slope, 8) + ",";
  json += "\"calibration_v_offset\":" + String(config.Voltage_Offset, 8) + ",";
  json += "\"calibration_i_slope\":" + String(config.Current_Slope, 8) + ",";
  json += "\"calibration_i_offset\":" + String(config.Current_Offset, 8) + ",";

  // WiFi settings
  json += "\"wifi_ssid\":\"" + String(config.WiFi_SSID) + "\",";
  // Do not send password
  // json += "\"wifi_password\":\"" + String(config.WiFi_Password) + "\",";
  json += "\"api_endpoint_url\":\"" + String(config.API_Endpoint_URL) + "\"";

  json += "}";

  return json;
}

void printConfig(const UserConfiguration &pconfig) {
  Log.noticeln(" ============ Configuration Details ============");
  Log.noticeln(" Logger ID: %u", pconfig.logger_id);
  Log.noticeln(" Cell ID: %u", pconfig.cell_id);
  Log.noticeln(" Upload Method: %s",
               pconfig.Upload_method == Uploadmethod_LoRa ? "LoRa" : "WiFi");
  Log.noticeln(" Upload Interval: %u sec", pconfig.Upload_interval);

  Log.noticeln(" Enabled Sensors (%d):", pconfig.enabled_sensors_count);
  for (int i = 0; i < pconfig.enabled_sensors_count; i++) {
    const char *sensor_name = "Unknown";
    switch (pconfig.enabled_sensors[i]) {
      case EnabledSensor_Voltage:
        sensor_name = "Voltage";
        break;
      case EnabledSensor_Current:
        sensor_name = "Current";
        break;
      case EnabledSensor_Teros12:
        sensor_name = "Teros12";
        break;
      case EnabledSensor_Teros21:
        sensor_name = "Teros21";
        break;
      case EnabledSensor_BME280:
        sensor_name = "BME280";
        break;
    }
    Log.noticeln("   - %s", sensor_name);
  }

  Log.noticeln(" Calibration Data:");
  char floatBuf[32];
  snprintf(floatBuf, sizeof(floatBuf), "%.4f", pconfig.Voltage_Slope);
  Log.noticeln("   Voltage Slope: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", pconfig.Voltage_Offset);
  Log.noticeln("   Voltage Offset: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", pconfig.Current_Slope);
  Log.noticeln("   Current Slope: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", pconfig.Current_Offset);
  Log.noticeln("   Current Offset: %s", floatBuf);

  Log.noticeln(" WiFi Settings:");
  Log.noticeln("   SSID: %s", pconfig.WiFi_SSID);
  Log.noticeln("   Password: %s", pconfig.WiFi_Password);
  Log.noticeln("   API Endpoint: %s", pconfig.API_Endpoint_URL);
  Log.noticeln(" =============================");
}

void printReceivedConfig() { printConfig(config); }
