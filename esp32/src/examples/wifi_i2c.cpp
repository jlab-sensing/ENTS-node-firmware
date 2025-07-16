#include <Arduino.h>
#include <Wire.h>
#include <ArduinoLog.h>
#include "modules/wifi.hpp"
#include "transcoder.h"
#include "pb_encode.h"
#include "pb_decode.h"

// I2C setup
const uint8_t stm32_addr = 0x20;
const int sda_pin = 0;
const int scl_pin = 1;
uint8_t tx_buf[128];
uint8_t rx_buf[256];
uint8_t encoded_user_config[256];
size_t encoded_len = 0;

// Callback to extract config_data bytes
bool config_data_callback(pb_istream_t *stream, const pb_field_t *field, void **arg) {
  uint8_t *buffer = (uint8_t *)(*arg);
  size_t size = stream->bytes_left;

  if (size > sizeof(encoded_user_config)) return false;
  if (!pb_read(stream, buffer, size)) return false;

  encoded_len = size;
  return true;
}

void sendUserConfigRequest() {
  Esp32Command cmd = Esp32Command_init_zero;
  cmd.which_command = Esp32Command_user_config_command_tag;
  cmd.command.user_config_command.type = UserConfigCommand_RequestType_REQUEST_CONFIG;

  pb_ostream_t ostream = pb_ostream_from_buffer(tx_buf, sizeof(tx_buf));
  if (!pb_encode(&ostream, Esp32Command_fields, &cmd)) {
    Log.errorln("Encoding failed: %s", PB_GET_ERROR(&ostream));
    return;
  }

  size_t tx_len = ostream.bytes_written;
  Log.noticeln("Sending request (%d bytes)", tx_len);

  Wire.beginTransmission(stm32_addr);
  Wire.write(tx_buf, tx_len);
  Wire.endTransmission();
}

bool readResponse() {
  Wire.requestFrom(stm32_addr, sizeof(rx_buf));
  size_t len = Wire.readBytes(rx_buf, sizeof(rx_buf));
  Log.noticeln("Received %d bytes", len);

  if (len == 0) {
    Log.errorln("No response from STM32");
    return false;
  }

  Esp32Command cmd = Esp32Command_init_zero;

  // Setup decode callback for config_data
  cmd.command.user_config_command.config_data.funcs.decode = config_data_callback;
  cmd.command.user_config_command.config_data.arg = encoded_user_config;

  pb_istream_t istream = pb_istream_from_buffer(rx_buf, len);
  if (!pb_decode(&istream, Esp32Command_fields, &cmd)) {
    Log.errorln("Decode error: %s", PB_GET_ERROR(&istream));
    return false;
  }

  if (cmd.which_command != Esp32Command_user_config_command_tag ||
      cmd.command.user_config_command.type != UserConfigCommand_RequestType_RESPONSE_CONFIG) {
    Log.errorln("Unexpected command received");
    return false;
  }

  UserConfiguration config = UserConfiguration_init_zero;
  pb_istream_t uc_stream = pb_istream_from_buffer(encoded_user_config, encoded_len);
  if (!pb_decode(&uc_stream, UserConfiguration_fields, &config)) {
    Log.errorln("Failed to decode user config");
    return false;
  }

  // Print config
  Log.noticeln("Logger ID: %u", config.logger_id);
  Log.noticeln("Cell ID: %u", config.cell_id);
  Log.noticeln("Upload method: %s", config.Upload_method == Uploadmethod_LoRa ? "LoRa" : "WiFi");
  Log.noticeln("Upload interval: %u", config.Upload_interval);
  Log.noticeln("WiFi SSID: %s", config.WiFi_SSID);
  Log.noticeln("API URL: %s", config.API_Endpoint_URL);
  Log.noticeln("Voltage slope: %.2f", config.Voltage_Slope);
  Log.noticeln("Current offset: %.2f", config.Current_Offset);

  return true;
}

void setup() {
  Serial.begin(115200);
  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  Wire.begin(sda_pin, scl_pin, 100000);
  delay(500);

  sendUserConfigRequest();
  delay(100);
  readResponse();  
}

void loop() {}
