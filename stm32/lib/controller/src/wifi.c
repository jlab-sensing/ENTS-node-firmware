#include "controller/wifi.h"

#include "communication.h"
#include "transcoder.h"

/** Timeout for i2c communication with esp32 */
unsigned int g_controller_i2c_timeout = 10000;

uint8_t ControllerWiFiInit(const char* ssid, const char* passwd) {
  // get reference to tx and rx buffers
  Buffer* tx = ControllerTx();
  Buffer* rx = ControllerRx();

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CONNECT;
  strncpy(wifi_cmd.ssid, ssid, sizeof(wifi_cmd.ssid));
  strncpy(wifi_cmd.passwd, passwd, sizeof(wifi_cmd.passwd));

  // encode command
  tx->len = EncodeWiFiCommand(&wifi_cmd, tx->data, tx->size);

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

  // return timestamp
  return cmd.command.wifi_command.rc;
}

uint32_t ControllerWiFiTime(void) {
  // get reference to tx and rx buffers
  Buffer* tx = ControllerTx();
  Buffer* rx = ControllerRx();

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_TIME;

  tx->len = EncodeWiFiCommand(&wifi_cmd, tx->data, tx->size);

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

  // return timestamp
  return cmd.command.wifi_command.ts;
}

unsigned int ControllerWiFiCheck(const char* url, const uint32_t port) {
  // get reference to tx and rx buffers
  Buffer* tx = ControllerTx();
  Buffer* rx = ControllerRx();

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK;
  strncpy(wifi_cmd.url, url, sizeof(wifi_cmd.url));
  wifi_cmd.port = port;

  tx->len = EncodeWiFiCommand(&wifi_cmd, tx->data, tx->size);

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

  // return timestamp
  return cmd.command.wifi_command.rc;
}

int ControllerWiFiPost(const uint8_t* data, size_t data_len, uint8_t* resp,
                       uint8_t* resp_len) {
  // return code
  int rc = 0;

  // get reference to tx and rx buffers
  Buffer* tx = ControllerTx();
  Buffer* rx = ControllerRx();

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_POST;
  memcpy(wifi_cmd.resp.bytes, data, data_len);
  wifi_cmd.resp.size = data_len;

  // encode command
  tx->len = EncodeWiFiCommand(&wifi_cmd, tx->data, tx->size);

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
  rc = cmd.command.wifi_command.rc;

  // copy response if not empty
  *resp_len = cmd.command.wifi_command.resp.size;
  if (*resp_len > 0) {
    memcpy(resp, cmd.command.wifi_command.resp.bytes,
           cmd.command.wifi_command.resp.size);
  }

  // return http code
  return rc;
}
