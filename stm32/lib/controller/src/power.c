#include "communication.h"
#include "controller/wifi.h"
#include "transcoder.h"

/** Timeout for i2c communication with esp32, in communication.h */
extern unsigned int g_controller_i2c_timeout;

ControllerStatus PowerCommandTransaction(const PowerCommand *input,
                                         PowerCommand *output);

ControllerStatus PowerCommandTransaction(const PowerCommand *input,
                                         PowerCommand *output) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // encode command
  tx->len = EncodePowerCommand(input, tx->data, tx->size);

  // send transaction
  ControllerStatus status = CONTROLLER_SUCCESS;
  status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    return status;
  }

  // check for errors
  if (rx->len == 0) {
    return CONTROLLER_ERROR;
  }

  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  cmd = DecodeEsp32Command(rx->data, rx->len);

  // copy output data
  memcpy(output, &cmd.command.wifi_command, sizeof(PowerCommand));

  return CONTROLLER_SUCCESS;
}

bool ControllerPowerSleep(void) {
  // format input command
  PowerCommand power_cmd = PowerCommand_init_zero;
  power_cmd.type = PowerCommand_Type_SLEEP;

  PowerCommand resp = PowerCommand_init_zero;

  if (PowerCommandTransaction(&power_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

bool ControllerPowerWakeup(void) {
  PowerCommand power_cmd = PowerCommand_init_zero;
  power_cmd.type = PowerCommand_Type_WAKEUP;

  PowerCommand resp = PowerCommand_init_zero;

  if (PowerCommandTransaction(&power_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  // NOTE can handle boot number/reason from resp if needed

  return true;
}
