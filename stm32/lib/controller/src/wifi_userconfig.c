#include "controller/wifi_userconfig.h"

#include "communication.h"
#include "transcoder.h"
#include "userConfig.h"

// Static variable for I2C timeout (same as WiFi)
static unsigned int g_controller_i2c_timeout = 10000;

UserConfigStatus ControllerUserConfigRequest(void) {
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // Clear buffers
  memset(tx->data, 0, tx->size);
  memset(rx->data, 0, rx->size);
  tx->len = 0;
  rx->len = 0;
  // Encode and send request
  tx->len = EncodeUserConfigCommand(
      UserConfigCommand_RequestType_REQUEST_CONFIG, NULL, tx->data, tx->size);
  if (tx->len == 0) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config request\r\n");
    return USERCONFIG_ENCODE_ERROR;
  }

  ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    APP_LOG(TS_OFF, VLEVEL_M, "Config request failed: %d\r\n", status);
    return USERCONFIG_COMM_ERROR;
  }

  // Check if we got a response
  if (rx->len == 0) {
    APP_LOG(TS_OFF, VLEVEL_M, "Empty config response\r\n");
    return USERCONFIG_NO_RESPONSE;
  }

  // Decode the response
  Esp32Command cmd = DecodeEsp32Command(rx->data, rx->len);
  if (cmd.which_command != Esp32Command_user_config_command_tag) {
    APP_LOG(TS_OFF, VLEVEL_M, "Invalid response type\r\n");
    return USERCONFIG_INVALID_RESPONSE;
  }

  if (cmd.command.user_config_command.type ==
      UserConfigCommand_RequestType_RESPONSE_CONFIG) {
    if (cmd.command.user_config_command.has_config_data) {
      const UserConfiguration *config =
          &cmd.command.user_config_command.config_data;
      // Check if config is all zeros (uninitialized)
      if (isConfigEmpty(config)) {
        APP_LOG(TS_OFF, VLEVEL_M, "Received empty config from ESP32\r\n");
        return USERCONFIG_EMPTY_CONFIG;
      }

      if (UserConfigSave(config)) {
        APP_LOG(TS_OFF, VLEVEL_M, "Failed to save config to FRAM\r\n");
        return USERCONFIG_FRAM_ERROR;
      }

      return USERCONFIG_OK;
    }
  }

  APP_LOG(TS_OFF, VLEVEL_M, "Invalid config response format\r\n");
  return USERCONFIG_INVALID_RESPONSE;
}

UserConfigStatus ControllerUserConfigSend(void) {
  const UserConfiguration *config = UserConfigGet();
  if (config == NULL) {
    APP_LOG(TS_OFF, VLEVEL_M, "Null config received from FRAM\r\n");
    return USERCONFIG_NULL_CONFIG;
  }

  // Print config being sent
  APP_LOG(
      TS_OFF, VLEVEL_M,
      "-----------------Sending configuration to ESP32-----------------:\r\n");
  UserConfigPrintAny(config);

  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // Clear buffers
  memset(tx->data, 0, tx->size);
  memset(rx->data, 0, rx->size);
  tx->len = 0;
  rx->len = 0;

  // Prepare response
  UserConfigCommand response = {0};
  response.type = UserConfigCommand_RequestType_RESPONSE_CONFIG;
  response.has_config_data = true;
  memcpy(&response.config_data, config, sizeof(UserConfiguration));

  // Encode and send
  tx->len = EncodeUserConfigCommand(response.type, &response.config_data,
                                    tx->data, tx->size);
  if (tx->len == 0) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config response\r\n");
    return USERCONFIG_ENCODE_ERROR;
  }

  ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config: %d\r\n", status);
    return USERCONFIG_COMM_ERROR;
  }

  APP_LOG(TS_OFF, VLEVEL_M, "Configuration successfully sent to ESP32\r\n");
  return USERCONFIG_OK;
}

bool ControllerUserConfigStart(void) {
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // Clear buffers
  memset(tx->data, 0, tx->size);
  memset(rx->data, 0, rx->size);
  tx->len = 0;
  rx->len = 0;

  UserConfigCommand cmd = UserConfigCommand_init_zero;
  cmd.type = UserConfigCommand_RequestType_START;

  tx->len =
      EncodeUserConfigCommand(cmd.type, &cmd.config_data, tx->data, tx->size);
  if (tx->len == 0) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config response\r\n");
    return false;
  }

  ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to send start request: %d\r\n", status);
    return false;
  }

  return true;
}

bool isConfigEmpty(const UserConfiguration *config) {
  // Check if logger_id & cell_id fields are zero/default
  return (config->logger_id == 0 && config->cell_id == 0);
}
