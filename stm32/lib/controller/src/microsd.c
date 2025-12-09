#include "controller/microsd.h"

#include "communication.h"
#include "transcoder.h"

/** Timeout for i2c communication with esp32, in communication.h */
extern unsigned int g_controller_i2c_timeout;

uint32_t ControllerMicroSDSave(const uint8_t *data, const uint16_t num_bytes) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_SAVE;

  microsd_cmd.which_data = MicroSDCommand_meas_tag;
  // Data is already Measurement-encoded.
  // Therefore, decode it into microsd_cmd and re-encode altogether.
  if (DecodeMeasurement(&microsd_cmd.data.meas, data, num_bytes) == -1) {
    APP_LOG(TS_OFF, VLEVEL_M,
            "Failed to decode measurement in ControllerMicroSDSave().\r\n");
  }

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

  switch (cmd.command.microsd_command.rc) {
    case MicroSDCommand_ReturnCode_SUCCESS:
      APP_LOG(TS_OFF, VLEVEL_L,
              "Successfully saved measurement to microSD card.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_GENERAL:
      APP_LOG(TS_OFF, VLEVEL_M, "Error: General error.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_MICROSD_NOT_INSERTED:
      APP_LOG(TS_OFF, VLEVEL_M, "Error: MicroSD card not inserted.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_FILE_SYSTEM_NOT_MOUNTABLE:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: File system on microSD card not mountable.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_PAYLOAD_NOT_DECODED:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: Payload in sent message not decodable.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: File(s) on microSD card not openable.\r\n");
      break;
    default:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: Unknown MicroSDCommand_ReturnCode (%d).\r\n",
              cmd.command.microsd_command.rc);
      break;
  }

  return cmd.command.microsd_command.rc;
}

uint32_t ControllerMicroSDUserConfig(UserConfiguration *uc,
                                     const char *filename) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_USERCONFIG;

  // Prepend current timestamp to filename, 32-bit max is 10 digits long
  // Ex. If input filename is "data.csv", sent filename is
  // "/<timestamp>_data.csv" Userconfig will be written to
  // "/<timestamp>_data.csv.userconfig"
  snprintf(microsd_cmd.filename, sizeof(microsd_cmd.filename), "/%d_%s",
           SysTimeGet().Seconds, filename);

  APP_LOG(TS_OFF, VLEVEL_M, "filename sent to ESP32: %s\r\n",
          microsd_cmd.filename);

  microsd_cmd.which_data = MicroSDCommand_uc_tag;
  memcpy(&microsd_cmd.data.uc, uc, sizeof(UserConfiguration));

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

  switch (cmd.command.microsd_command.rc) {
    case MicroSDCommand_ReturnCode_SUCCESS:
      APP_LOG(
          TS_OFF, VLEVEL_L,
          "Successfully saved userConfig and CSV headers to microSD card.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_GENERAL:
      APP_LOG(TS_OFF, VLEVEL_M, "Error: General error.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_MICROSD_NOT_INSERTED:
      APP_LOG(TS_OFF, VLEVEL_M, "Error: MicroSD card not inserted.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_FILE_SYSTEM_NOT_MOUNTABLE:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: File system on microSD card not mountable.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_PAYLOAD_NOT_DECODED:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: Payload in sent message not decodable.\r\n");
      break;
    case MicroSDCommand_ReturnCode_ERROR_FILE_NOT_OPENED:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: File(s) on microSD card not openable.\r\n");
      break;
    default:
      APP_LOG(TS_OFF, VLEVEL_M,
              "Error: Unknown MicroSDCommand_ReturnCode (%d).\r\n",
              cmd.command.microsd_command.rc);
      break;
  }

  return cmd.command.microsd_command.rc;
}
