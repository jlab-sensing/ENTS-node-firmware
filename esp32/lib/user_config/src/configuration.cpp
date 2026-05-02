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
  config.enabled_sensors_multiple_count =
      new_config.enabled_sensors_multiple_count;
  memcpy(config.enabled_sensors_multiple, new_config.enabled_sensors_multiple,
         sizeof(config.enabled_sensors_multiple));

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
  json += "\"upload_method\":\"";
  json += Uploadmethod_name(config.Upload_method);
  json += "\",";
  json += "\"upload_interval\":" + String(config.Upload_interval) + ",";

  // Enabled sensors
  json += "\"enabled_sensors_multiple_count\":" +
          String(config.enabled_sensors_multiple_count) + ",";
  json += "\"enabled_sensors_multiple\":[";
  for (uint8_t i = 0; i < config.enabled_sensors_multiple_count; i++) {
    if (i > 0) json += ",";

    json += "{\"";
    json +=
        EnabledSensor_name(config.enabled_sensors_multiple[i].enabled_sensor);
    json += ",";
    json += String(config.enabled_sensors_multiple[i].cell_id) + ",";
    json += String(config.enabled_sensors_multiple[i].index) + ",";
    json += "\"}";
  }
  json += "],";

  char calib_buffer[32];
  snprintf(calib_buffer, sizeof(calib_buffer), "%g", config.Voltage_Slope);
  json += "\"calibration_v_slope\":" + String(calib_buffer) + ",";
  snprintf(calib_buffer, sizeof(calib_buffer), "%g", config.Voltage_Offset);
  json += "\"calibration_v_offset\":" + String(calib_buffer) + ",";
  snprintf(calib_buffer, sizeof(calib_buffer), "%g", config.Current_Slope);
  json += "\"calibration_i_slope\":" + String(calib_buffer) + ",";
  snprintf(calib_buffer, sizeof(calib_buffer), "%g", config.Current_Offset);
  json += "\"calibration_i_offset\":" + String(calib_buffer) + ",";

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
  Log.noticeln(" Upload Method: %s", Uploadmethod_name(pconfig.Upload_method));
  Log.noticeln(" Upload Interval: %u sec", pconfig.Upload_interval);

  Log.noticeln(" Enabled Sensors Multiple (%d):",
               pconfig.enabled_sensors_multiple_count);
  for (int i = 0; i < pconfig.enabled_sensors_multiple_count; i++) {
    Log.noticeln(
        " %03d - %s\r\n", i,
        EnabledSensor_name(pconfig.enabled_sensors_multiple[i].enabled_sensor));
    Log.noticeln(" %03d - cell_id=%d\r\n", i,
                 pconfig.enabled_sensors_multiple[i].cell_id);
    Log.noticeln(" %03d - index=%d\r\n", i,
                 pconfig.enabled_sensors_multiple[i].index);
  }

  // NOTE(jtmadden): This doesn't print all the precision
  Log.noticeln(" Calibration Data:");
  Log.noticeln("   Voltage Slope: %.9lf", pconfig.Voltage_Slope);
  Log.noticeln("   Voltage Offset: %.9lf", pconfig.Voltage_Offset);
  Log.noticeln("   Current Slope: %.9lf", pconfig.Current_Slope);
  Log.noticeln("   Current Offset: %.9lf", pconfig.Current_Offset);

  Log.noticeln(" WiFi Settings:");
  Log.noticeln("   SSID: %s", pconfig.WiFi_SSID);
  Log.noticeln("   Password: %s", pconfig.WiFi_Password);
  Log.noticeln("   API Endpoint: %s", pconfig.API_Endpoint_URL);
  Log.noticeln(" =============================");
}

void printReceivedConfig() { printConfig(config); }
