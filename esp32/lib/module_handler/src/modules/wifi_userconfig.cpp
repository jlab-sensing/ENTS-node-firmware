#include "modules/wifi_userconfig.hpp"

#include <pb_decode.h>
#include <pb_encode.h>
#include <string.h>

#include "config_server.hpp"
#include "configuration.hpp"

namespace ModuleHandler {

ModuleUserConfig::ModuleUserConfig() : Module() {
  this->type = Esp32Command_user_config_command_tag;
  this->state = 0;
  memset(&current_command_, 0, sizeof(current_command_));
  memset(&current_config_, 0, sizeof(current_config_));
}

void ModuleUserConfig::OnReceive(const Esp32Command &cmd) {
  // Check which command
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

    setConfig(current_config_);
    //updateWebConfig(&current_config_);
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
}  // namespace ModuleHandler
