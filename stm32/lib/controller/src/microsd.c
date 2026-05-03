#include "controller/microsd.h"

#include "communication.h"
#include "sensor.h"
#include "transcoder.h"

/** Timeout for i2c communication with esp32, in communication.h */
extern unsigned int g_controller_i2c_timeout;

MicroSDCommand_ReturnCode ControllerMicroSDSave(const uint8_t *data,
                                                const uint16_t num_bytes) {
  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  MicroSDCommand microsd_cmd = MicroSDCommand_init_zero;
  microsd_cmd.type = MicroSDCommand_Type_SAVE;

  microsd_cmd.which_data = MicroSDCommand_sensor_measurement_tag;
  // Data is already Measurement-encoded.
  // Therefore, decode it into microsd_cmd and re-encode altogether.
  if (DecodeSensorMeasurement(data, num_bytes,
                              &microsd_cmd.data.sensor_measurement) == -1) {
    APP_LOG(
        TS_OFF, VLEVEL_H,
        "Failed to decode sensor measurement in ControllerMicroSDSave().\r\n");
  }

  // encode command
  tx->len = EncodeMicroSDCommand(&microsd_cmd, tx->data, tx->size);
  APP_LOG(TS_OFF, VLEVEL_H, "Encoded microsd Command (%d): 0x", tx->len);
  for (int i = 0; i < tx->len; i++) {
    APP_LOG(TS_OFF, VLEVEL_H, "%02X", tx->data[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_H, "\r\n");

  // return if communication fails
  ControllerStatus status = CONTROLLER_SUCCESS;
  status = ControllerTransaction(g_controller_i2c_timeout);
  if (status != CONTROLLER_SUCCESS) {
    APP_LOG(TS_OFF, VLEVEL_M, "ControllerTransaction() status error (%d)\r\n",
            status);  // see ControllerStatus
    return MicroSDCommand_ReturnCode_ERROR_GENERAL;
  }

  // check for errors
  if (rx->len == 0) {
    APP_LOG(TS_OFF, VLEVEL_M,
            "ControllerTransaction() receive length error, receive length was "
            "%d\r\n",
            rx->len);
    return MicroSDCommand_ReturnCode_ERROR_GENERAL;
  }

  // decode command
  Esp32Command esp32_response = Esp32Command_init_default;
  esp32_response = DecodeEsp32Command(rx->data, rx->len);

  APP_LOG(TS_OFF, VLEVEL_M, "%s\r\n",
          MicroSDCommand_ReturnCode_name(
              esp32_response.command.microsd_command.rc));
  if (esp32_response.command.microsd_command.rc !=
      MicroSDCommand_ReturnCode_SUCCESS) {
    // Additional error messages may be included in the data.raw_data field as
    // strings. Check for valid string (null-terminated).
    for (int i = esp32_response.command.microsd_command.data.raw_data.size;
         i > 0; i--) {
      if (esp32_response.command.microsd_command.data.raw_data.bytes[i] ==
          '\0') {
        APP_LOG(TS_OFF, VLEVEL_M,
                "I2C received raw_data field contains: %s\r\n",
                esp32_response.command.microsd_command.data.raw_data.bytes);
      }
    }
  }

  return esp32_response.command.microsd_command.rc;
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
  snprintf(microsd_cmd.filename, sizeof(microsd_cmd.filename), "/%ld_%s",
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
