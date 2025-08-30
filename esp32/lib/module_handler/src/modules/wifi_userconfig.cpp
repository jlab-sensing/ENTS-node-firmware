#include "modules/wifi_userconfig.hpp"

#include <pb_decode.h>
#include <pb_encode.h>
#include <string.h>

#include "configuration.hpp"
#include "config_server.hpp"

namespace ModuleHandler {

ModuleUserConfig::ModuleUserConfig() : Module() {
  this->type = Esp32Command_user_config_command_tag;
  this->state = 0;

  UserConfiguration default_config = UserConfiguration_init_default;
  strncpy(default_config.API_Endpoint_URL, "http://dirtviz.jlab.ucsc.edu/api/sensor/", sizeof(default_config.API_Endpoint_URL));
  setConfig(default_config);
}

void ModuleUserConfig::OnReceive(const Esp32Command &cmd) {
  Log.traceln("ModuleUserConfig::OnReceive");

  // Check which command
  if (cmd.which_command != Esp32Command_user_config_command_tag) {
    Log.errorln(" Wrong command type received");
    return;
  }

  switch (cmd.command.user_config_command.type) {
    case UserConfigCommand_RequestType_REQUEST_CONFIG:
      Log.traceln("Received config request");
      requestConfig(cmd.command.user_config_command);
      break;

    case UserConfigCommand_RequestType_RESPONSE_CONFIG:
      Log.traceln("Received config response");
      responseConfig(cmd.command.user_config_command);
      break;

    case UserConfigCommand_RequestType_START:
      Log.traceln("Received start command");
      start();
      break;

    default:
      Log.errorln("Unknown user config command type: %d",
                  cmd.command.user_config_command.type);
      return;
  }
}

size_t ModuleUserConfig::OnRequest(uint8_t *buffer) {
  Log.traceln("ModuleUserConfig::OnRequest");
  memcpy(buffer, this->buffer, buffer_len);
  return buffer_len;
}


  void ModuleUserConfig::requestConfig(const UserConfigCommand &cmd) {
    Log.noticeln(" ============ Received Config Request ============");
    Log.noticeln(" STM32 is requesting current configuration");
    //Log.noticeln("============ Sending Configuration ============");
    //Log.noticeln(" Preparing to send current config to STM32");

    Esp32Command response = {0};
    response.which_command = Esp32Command_user_config_command_tag;
    response.command.user_config_command.type =
        UserConfigCommand_RequestType_RESPONSE_CONFIG;
    response.command.user_config_command.has_config_data = true;

    const UserConfiguration config = getConfig();

    memcpy(&response.command.user_config_command.config_data,
           &config, sizeof(UserConfiguration));

    pb_ostream_t ostream = pb_ostream_from_buffer(buffer, Esp32Command_size);
    if (!pb_encode(&ostream, Esp32Command_fields, &response)) {
      Log.errorln("Failed to encode response");
      buffer_len = 0;
    }

    buffer_len = ostream.bytes_written;

    Log.noticeln("Successfully encoded configuration (%d bytes)",
                 buffer_len);
  }

  void ModuleUserConfig::responseConfig(const UserConfigCommand &cmd) {
    Log.noticeln(" ============ Received New Configuration ============");

    setConfig(cmd.config_data);
    //updateWebConfig(&current_config_);
    printReceivedConfig();
  }

void ModuleUserConfig::start() {
  Log.noticeln("Starting WiFi User Config module");

  setupServer();
  Log.noticeln("HTTP server started");

  Esp32Command response = Esp32Command_init_zero;
  response.which_command = Esp32Command_user_config_command_tag;
  response.command.user_config_command.type =
    UserConfigCommand_RequestType_START;

  pb_ostream_t ostream = pb_ostream_from_buffer(buffer, Esp32Command_size);
  if (!pb_encode(&ostream, Esp32Command_fields, &response)) {
    Log.errorln("Failed to encode response");
    buffer_len = 0;;
  }

  buffer_len = ostream.bytes_written;
}
}  // namespace ModuleHandler
