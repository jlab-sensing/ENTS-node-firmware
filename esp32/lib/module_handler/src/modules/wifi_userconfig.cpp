#include "modules/wifi_userconfig.hpp"

#include <pb_decode.h>
#include <pb_encode.h>
#include <string.h>

#include "config_server.h"

namespace ModuleHandler {

ModuleUserConfig::ModuleUserConfig() : Module() {
  this->type = Esp32Command_user_config_command_tag;
  this->state = 0;
  memset(&current_command_, 0, sizeof(current_command_));
  memset(&current_config_, 0, sizeof(current_config_));
}

void ModuleUserConfig::OnReceive(const Esp32Command &cmd) {
  if (cmd.which_command != Esp32Command_user_config_command_tag) {
    Log.errorln(" Wrong command type received");
    return;
  }

  memcpy(&current_command_, &cmd.command.user_config_command,
         sizeof(UserConfigCommand));

  if (current_command_.type == UserConfigCommand_RequestType_REQUEST_CONFIG) {
    Log.noticeln(" ============ Received Config Request ============");
    Log.noticeln(" STM32 is requesting current configuration");
  } else if (current_command_.type ==
                 UserConfigCommand_RequestType_RESPONSE_CONFIG &&
             current_command_.has_config_data) {
    Log.noticeln(" ============ Received New Configuration ============");
    memcpy(&current_config_, &current_command_.config_data,
           sizeof(UserConfiguration));
    has_config_ = true;

    updateWebConfig(&current_config_);
    printReceivedConfig();
  }
}

size_t ModuleUserConfig::OnRequest(uint8_t *buffer) {
  if (current_command_.type == UserConfigCommand_RequestType_REQUEST_CONFIG) {
    Log.noticeln("============ Sending Configuration ============");
    Log.noticeln(" Preparing to send current config to STM32");

    if (has_config_) {
      Log.noticeln(" Current configuration exists, sending it");
      printReceivedConfig();  // Print what we're about to send
    } else {
      Log.warningln(" No configuration available to send");
    }

    Esp32Command response = {0};
    response.which_command = Esp32Command_user_config_command_tag;
    response.command.user_config_command.type =
        UserConfigCommand_RequestType_RESPONSE_CONFIG;
    response.command.user_config_command.has_config_data = has_config_;

    if (has_config_) {
      memcpy(&response.command.user_config_command.config_data,
             &current_config_, sizeof(UserConfiguration));
    }

    pb_ostream_t stream = pb_ostream_from_buffer(buffer, Esp32Command_size);
    if (!pb_encode(&stream, Esp32Command_fields, &response)) {
      Log.errorln("Failed to encode response");
      return 0;
    }

    Log.noticeln(" Successfully encoded configuration (%d bytes)",
                 stream.bytes_written);
    return stream.bytes_written;
  }
  return 0;
}

void ModuleUserConfig::printReceivedConfig() {
  Log.noticeln(" ============ Configuration Details ============");
  Log.noticeln(" Logger ID: %u", current_config_.logger_id);
  Log.noticeln(" Cell ID: %u", current_config_.cell_id);
  Log.noticeln(
      " Upload Method: %s",
      current_config_.Upload_method == Uploadmethod_LoRa ? "LoRa" : "WiFi");
  Log.noticeln(" Upload Interval: %u sec", current_config_.Upload_interval);

  Log.noticeln(" Enabled Sensors (%d):", current_config_.enabled_sensors_count);
  for (int i = 0; i < current_config_.enabled_sensors_count; i++) {
    const char *sensor_name = "Unknown";
    switch (current_config_.enabled_sensors[i]) {
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
  snprintf(floatBuf, sizeof(floatBuf), "%.4f", current_config_.Voltage_Slope);
  Log.noticeln("   Voltage Slope: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", current_config_.Voltage_Offset);
  Log.noticeln("   Voltage Offset: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", current_config_.Current_Slope);
  Log.noticeln("   Current Slope: %s", floatBuf);

  snprintf(floatBuf, sizeof(floatBuf), "%.4f", current_config_.Current_Offset);
  Log.noticeln("   Current Offset: %s", floatBuf);

  if (current_config_.Upload_method == Uploadmethod_WiFi) {
    Log.noticeln(" WiFi Settings:");
    Log.noticeln("   SSID: %s", current_config_.WiFi_SSID);
    Log.noticeln("   Password: %s", current_config_.WiFi_Password);
    Log.noticeln("   API Endpoint: %s:%u", current_config_.API_Endpoint_URL,
                 current_config_.API_Endpoint_Port);
  }
  Log.noticeln(" =============================");
}
void ModuleUserConfig::updateWebConfig(const UserConfiguration *pb_config) {
  // Convert protobuf config to web server format
  config.logger_id = pb_config->logger_id;
  config.cell_id = pb_config->cell_id;
  config.upload_interval = pb_config->Upload_interval;

  config.upload_method =
      (pb_config->Upload_method == Uploadmethod_LoRa) ? "LoRa" : "WiFi";

  // Update enabled sensors
  config.voltage_enabled = false;
  config.current_enabled = false;
  config.teros12_enabled = false;
  config.teros21_enabled = false;
  config.bme280_enabled = false;

  for (int i = 0; i < pb_config->enabled_sensors_count; i++) {
    switch (pb_config->enabled_sensors[i]) {
      case EnabledSensor_Voltage:
        config.voltage_enabled = true;
        break;
      case EnabledSensor_Current:
        config.current_enabled = true;
        break;
      case EnabledSensor_Teros12:
        config.teros12_enabled = true;
        break;
      case EnabledSensor_Teros21:
        config.teros21_enabled = true;
        break;
      case EnabledSensor_BME280:
        config.bme280_enabled = true;
        break;
    }
  }

  // Update calibration data
  config.calibration_v_slope = pb_config->Voltage_Slope;
  config.calibration_v_offset = pb_config->Voltage_Offset;
  config.calibration_i_slope = pb_config->Current_Slope;
  config.calibration_i_offset = pb_config->Current_Offset;

  // Update WiFi settings
  config.wifi_ssid = String(pb_config->WiFi_SSID);
  config.wifi_password = String(pb_config->WiFi_Password);
  config.api_endpoint_url = String(pb_config->API_Endpoint_URL);
  config.api_endpoint_port = pb_config->API_Endpoint_Port;

  Log.noticeln("Web server configuration updated");
}

}  // namespace ModuleHandler