#include "controller/irrigation.h"

#include "communication.h"
#include "transcoder.h"

extern unsigned int g_controller_i2c_timeout;

ControllerStatus IrrigationCommandTransaction(const IrrigationCommand *input,
                                              IrrigationCommand *output) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // encode command
  tx->len = EncodeIrrigationCommand(input, tx->data, tx->size);

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
  memcpy(output, &cmd.command.irrigation_command, sizeof(IrrigationCommand));

  return CONTROLLER_SUCCESS;
}

IrrigationCommand_State ControllerIrrigationCheck(void) {
  IrrigationCommand irrigation_cmd = IrrigationCommand_init_zero;
  irrigation_cmd.type = IrrigationCommand_Type_CHECK;

  IrrigationCommand resp = IrrigationCommand_init_zero;

  if (IrrigationCommandTransaction(&irrigation_cmd, &resp) !=
      CONTROLLER_SUCCESS) {
    return CONTROLLER_ERROR;
  }

  return resp.state;
}
