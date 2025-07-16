#include "userConfig.h"
#include "communication.h"
#include "transcoder.h"

// Static variable for I2C timeout (same as WiFi)
unsigned int g_controller_i2c_timeout = 10000;

/**
 * @brief Sends the current user configuration to the ESP32
 * @return USERCONFIG_OK on success, error code on failure
 */
UserConfigStatus ControllerUserConfigSend(void) {
  // Get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // Get the current configuration
  const UserConfiguration *config = UserConfigGet();
  if (config == NULL) {
      return USERCONFIG_NOT_LOADED;
  }

  // Create and initialize the command
  UserConfigCommand user_cmd = UserConfigCommand_init_zero;
  user_cmd.type = UserConfigCommand_RequestType_RESPONSE_CONFIG;
  user_cmd.has_config_data = true;
  memcpy(&user_cmd.config_data, config, sizeof(UserConfiguration));

  // Encode the command
  tx->len = EncodeUserConfigCommand(user_cmd.type, &user_cmd.config_data, tx->data, tx->size);

  // Send transaction
  ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
      return USERCONFIG_COMM_ERROR;
  }

  // Check for empty response
  if (rx->len == 0) {
      return USERCONFIG_NO_RESPONSE;
  }

  // Decode the response
  Esp32Command cmd = DecodeEsp32Command(rx->data, rx->len);

  // Verify we got a success response
  if (cmd.which_command != Esp32Command_user_config_command_tag ||
      cmd.command.user_config_command.type != UserConfigCommand_RequestType_RESPONSE_CONFIG) {
      return USERCONFIG_INVALID_RESPONSE;
  }

  return USERCONFIG_OK;
}