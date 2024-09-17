#include "controller/wifi.h"

#include "transcoder.h"
#include "communication.h"

uint32_t ControllerWiFiInit(
  const char* ssid,
  const char* passwd,
  const char* url,
  const char* port
) {
  // encode command
  tx.len = EncodeWiFiCommand(WiFiCommand_Type_CONNECT, ssid, passwd, url, port,
                             0, 0, NULL, 0, tx.data, tx.size)

  // send transaction 
  ControllerTransaction(500);

  // check for errors
  if (rx.len = 0) {
    return 0;
  }

  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  DecodeEsp32Command(rx.data, rx.len);

  // return timestamp
  return cmd.command.wifi_command.ts;
}

int ControllerWiFiPost(
  const uint8_t* data,
  size_t data_len,
  uint8_t* resp,
  uint8_t* resp_len
) {
  // encode command
  tx.len = EncodeWiFiCommand(WiFiCommand_Type_POST, NULL, NULL, NULL, 0, 0, 0,
                             data, data_len)

  // send transaction
  ControllerTransaction(500);

  // check for errors
  if (rx.len = 0) {
    return 0;
  }
  
  // decode command
  Esp32Command cmd = Esp32Command_init_default;
  cmd = DecodeEsp32Command(rx.data, rx.len);

  // copy response
  memcpy(resp, cmd.command.wifi_command.resp.bytes, cmd.command.wifi_command.resp.size);
  *resp_len = cmd.command.wifi_command.resp.size;

  // return http code
  return cmd.command.wifi_command.rc;
}