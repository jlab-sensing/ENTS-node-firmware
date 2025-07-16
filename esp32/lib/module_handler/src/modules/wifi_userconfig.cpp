#include "modules/wifi_userconfig.hpp"
#include <pb_decode.h>
#include <pb_encode.h>
#include <ArduinoLog.h>
#include <string.h>

namespace ModuleHandler {

ModuleUserConfig::ModuleUserConfig() : Module() {
    this->type = Esp32Command_user_config_command_tag;
    this->state = 0;
    memset(&current_command_, 0, sizeof(current_command_));
    memset(&current_config_, 0, sizeof(current_config_));
}

void ModuleUserConfig::OnReceive(const Esp32Command &cmd) {
  if (cmd.which_command != Esp32Command_user_config_command_tag) {
      Log.errorln("Wrong command type received");
      return;
  }

  memcpy(&current_command_, &cmd.command.user_config_command, sizeof(UserConfigCommand));

  if (current_command_.type == UserConfigCommand_RequestType_RESPONSE_CONFIG && 
      current_command_.has_config_data) {
      memcpy(&current_config_, &current_command_.config_data, sizeof(UserConfiguration));
      
      // Print complete received configuration
      Log.noticeln("==== Received Configuration ====");
      Log.noticeln("Logger ID: %u", current_config_.logger_id);
      Log.noticeln("Cell ID: %u", current_config_.cell_id);
      Log.noticeln("Upload Method: %s", 
                  current_config_.Upload_method == Uploadmethod_LoRa ? "LoRa" : "WiFi");
      Log.noticeln("Upload Interval: %u sec", current_config_.Upload_interval);
      
      // Print enabled sensors
      for (int i = 0; i < current_config_.enabled_sensors_count; i++) {
          const char *sensor_name = "Unknown";
          switch(current_config_.enabled_sensors[i]) {
              case EnabledSensor_Voltage: sensor_name = "Voltage"; break;
              case EnabledSensor_Current: sensor_name = "Current"; break;
              case EnabledSensor_Teros12: sensor_name = "Teros12"; break;
              case EnabledSensor_Teros21: sensor_name = "Teros21"; break;
              case EnabledSensor_BME280: sensor_name = "BME280"; break;
          }
          Log.noticeln("Sensor %d: %s", i+1, sensor_name);
      }
      
      // Print calibration data
      Log.noticeln("Voltage Slope: %.2f", current_config_.Voltage_Slope);
      Log.noticeln("Voltage Offset: %.2f", current_config_.Voltage_Offset);
      Log.noticeln("Current Slope: %.2f", current_config_.Current_Slope);
      Log.noticeln("Current Offset: %.2f", current_config_.Current_Offset);
      
      // Print WiFi details
      Log.noticeln("WiFi SSID: %s", current_config_.WiFi_SSID);
      Log.noticeln("API Endpoint: %s",current_config_.API_Endpoint_URL);
      Log.noticeln("API Endpoint: %u",current_config_.API_Endpoint_Port);
      Log.noticeln("==============================");
  }
}

size_t ModuleUserConfig::OnRequest(uint8_t *buffer) {
    // Only handle requests if we have a valid command
    if (current_command_.type == UserConfigCommand_RequestType_REQUEST_CONFIG) {
        Log.traceln("Received config request from STM32");
        
        Esp32Command response = {0};
        response.which_command = Esp32Command_user_config_command_tag;
        response.command.user_config_command.type = UserConfigCommand_RequestType_RESPONSE_CONFIG;
        response.command.user_config_command.has_config_data = false;

        pb_ostream_t stream = pb_ostream_from_buffer(buffer, Esp32Command_size);
        if (!pb_encode(&stream, Esp32Command_fields, &response)) {
            Log.errorln("Failed to encode response");
            return 0;
        }
        return stream.bytes_written;
    }
    return 0;
}

}
