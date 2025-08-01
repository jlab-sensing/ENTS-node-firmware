#include "controller/microsd.h"

#include "communication.h"
#include "transcoder.h"

/** Timeout for i2c communication with esp32, in communication.h */
extern unsigned int g_controller_i2c_timeout;

uint32_t ControllerMicroSDTime(const char *filename) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_TIME;
  strncpy(microsd_cmd.filename, filename, sizeof(microsd_cmd.filename));
  tx->len = EncodeMicroSDCommand(&microsd_cmd, tx->data, tx->size);

  // send transaction
  ControllerStatus status = CONTROLLER_SUCCESS;
  status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    return -1;
  }

  // check for errors
  if (rx->len == 0) {
    return -1;
  }

  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  cmd = DecodeEsp32Command(rx->data, rx->len);

  // return timestamp (ts is 0 if not found)
  return cmd.command.microsd_command.ts;
}

uint32_t ControllerMicroSDCheck(const char *filename) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_SIZE;
  strncpy(microsd_cmd.filename, filename, sizeof(microsd_cmd.filename));

  tx->len = EncodeMicroSDCommand(&microsd_cmd, tx->data, tx->size);

  // send transaction
  ControllerStatus status = CONTROLLER_SUCCESS;
  status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    return -1;
  }

  // check for errors
  if (rx->len == 0) {
    return -1;
  }

  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  cmd = DecodeEsp32Command(rx->data, rx->len);

  // return filesize (filesize is -1 if not found)
  return cmd.command.microsd_command.filesize;
}

uint32_t ControllerMicroSDSave(const char *filename, const uint8_t *data,
                               const uint16_t num_bytes) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_SAVE;
  strncpy(microsd_cmd.filename, filename, sizeof(microsd_cmd.filename));
  memcpy(microsd_cmd.resp.bytes, data, num_bytes);
  microsd_cmd.resp.size = num_bytes;

  // encode command
  tx->len = EncodeMicroSDCommand(&microsd_cmd, tx->data, tx->size);

  // return if communication fails
  ControllerStatus status = CONTROLLER_SUCCESS;
  status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    return 0;
  }

  // check for errors
  if (rx->len == 0) {
    return 0;
  }

  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  cmd = DecodeEsp32Command(rx->data, rx->len);

  // return filesize
  return cmd.command.microsd_command.filesize;
}

uint32_t ControllerMicroSDUserConfig(UserConfiguration *uc,
                                     const char *filename) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // UserConfiguration uc = UserConfiguration_init_zero;

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_USERCONFIG;
  strncpy(microsd_cmd.filename, filename, sizeof(microsd_cmd.filename));
  microsd_cmd.resp.size = EncodeUserConfiguration(uc, microsd_cmd.resp.bytes);

  // encode command
  tx->len = EncodeMicroSDCommand(&microsd_cmd, tx->data, tx->size);

  // return if communication fails
  ControllerStatus status = CONTROLLER_SUCCESS;
  status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    return 0;
  }

  // check for errors
  if (rx->len == 0) {
    return 0;
  }

  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  cmd = DecodeEsp32Command(rx->data, rx->len);

  // return filesize
  return cmd.command.microsd_command.filesize;
}
