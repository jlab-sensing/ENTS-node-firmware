#include "configuration.hpp"

#include <ArduinoLog.h>
#include <string.h>


/** Instance of user config served by the webserver */
static UserConfiguration config = UserConfiguration_init_default;

void setConfig(const UserConfiguration &new_config) {
    // deeeeep copy
    config.logger_id = new_config.logger_id;
    config.cell_id = new_config.cell_id;
    config.Upload_method = new_config.Upload_method;
    config.Upload_interval = new_config.Upload_interval;
    config.enabled_sensors_count = new_config.enabled_sensors_count;
    memcpy(config.enabled_sensors, new_config.enabled_sensors, sizeof(config.enabled_sensors));
    config.Voltage_Slope = new_config.Voltage_Slope;
    config.Voltage_Offset = new_config.Voltage_Offset;
    config.Current_Slope = new_config.Current_Slope;
    config.Current_Offset = new_config.Current_Offset;
    memcpy(config.WiFi_SSID, new_config.WiFi_SSID, sizeof(config.WiFi_SSID));
    memcpy(config.WiFi_Password, new_config.WiFi_Password, sizeof(config.WiFi_Password));
    memcpy(config.API_Endpoint_URL, new_config.API_Endpoint_URL, sizeof(config.API_Endpoint_URL));
    config.API_Endpoint_Port = new_config.API_Endpoint_Port;
  
    // Check that the user config can be encoded/decoded?? (John)
    uint8_t buffer[UserConfiguration_size];
    size_t message_length = EncodeUserConfiguration(&config, buffer);
    UserConfiguration decoded_config = UserConfiguration_init_zero;

    if (message_length > 0) {
        //printEncodedData(buffer, message_length);

        if (DecodeUserConfiguration(buffer, message_length, &decoded_config) == 0) {
            //printDecodedConfig(&decoded_config);
        } else {
            Log.errorln("Failed to decode the configuration");
        }
    } else {
        Log.errorln("Failed to encode the configuration");
    }
}

const UserConfiguration& getConfig() {
    return config;
}

void printReceivedConfig() {
  Log.noticeln(" ============ Configuration Details ============");
  Log.noticeln(" Logger ID: %u", config.logger_id);
  Log.noticeln(" Cell ID: %u", config.cell_id);
  Log.noticeln(
      " Upload Method: %s",
      config.Upload_method == Uploadmethod_LoRa ? "LoRa" : "WiFi");
  Log.noticeln(" Upload Interval: %u sec", config.Upload_interval);

  Log.noticeln(" Enabled Sensors (%d):", config.enabled_sensors_count);
  for (int i = 0; i < config.enabled_sensors_count; i++) {
    const char *sensor_name = "Unknown";
    switch (config.enabled_sensors[i]) {
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
  snprintf(floatBuf, sizeof(floatBuf), "%.4f", config.Voltage_Slope);
  Log.noticeln("   Voltage Slope: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", config.Voltage_Offset);
  Log.noticeln("   Voltage Offset: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", config.Current_Slope);
  Log.noticeln("   Current Slope: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", config.Current_Offset);
  Log.noticeln("   Current Offset: %s", floatBuf);

  if (config.Upload_method == Uploadmethod_WiFi) {
    Log.noticeln(" WiFi Settings:");
    Log.noticeln("   SSID: %s", config.WiFi_SSID);
    Log.noticeln("   Password: %s", config.WiFi_Password);
    Log.noticeln("   API Endpoint: %s:%u", config.API_Endpoint_URL,
                 config.API_Endpoint_Port);
  }
  Log.noticeln(" =============================");
}
