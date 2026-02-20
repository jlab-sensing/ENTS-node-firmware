#include "controller/wifi.h"

#include <assert.h>

#include "communication.h"
#include "transcoder.h"

/** Timeout for i2c communication with esp32, in communication.h */
extern unsigned int g_controller_i2c_timeout;

/** Flag to track if the WiFi controller has been initialized */
static bool wifi_initialized = false;

void ControllerWiFiSetInitialized(void) { wifi_initialized = true; }

ControllerStatus WiFiCommandTransaction(const WiFiCommand *input,
                                        WiFiCommand *output) {
  assert(wifi_initialized &&
         "WiFi controller used before ControllerWiFiSetInitialized() was "
         "called. Ensure WiFi is enabled in user config.");

  // get reference to tx and rx buffers
  Buffer *tx = ControllerTx();
  Buffer *rx = ControllerRx();

  // encode command
  tx->len = EncodeWiFiCommand(input, tx->data, tx->size);

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
  memcpy(output, &cmd.command.wifi_command, sizeof(WiFiCommand));

  return CONTROLLER_SUCCESS;
}

bool ControllerWiFiConnect(const char *ssid, const char *passwd) {
  // format input command
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CONNECT;
  strncpy(wifi_cmd.ssid, ssid, sizeof(wifi_cmd.ssid));
  strncpy(wifi_cmd.passwd, passwd, sizeof(wifi_cmd.passwd));

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

bool ControllerWiFiDisconnect(void) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_DISCONNECT;

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

ControllerWiFiStatus ControllerWiFiCheckWiFi(void) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK_WIFI;

  WiFiCommand resp = WiFiCommand_init_zero;

  WiFiCommandTransaction(&wifi_cmd, &resp);

  return resp.rc;
}

bool ControllerWiFiNtpSync(void) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_NTP_SYNC;

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

uint32_t ControllerWiFiTime(void) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_TIME;

  WiFiCommand resp = WiFiCommand_init_zero;

  WiFiCommandTransaction(&wifi_cmd, &resp);

  return resp.ts;
}

bool ControllerWiFiCheckApi(const char *url) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK_API;
  strncpy(wifi_cmd.url, url, sizeof(wifi_cmd.url));

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

bool ControllerWiFiPost(const uint8_t *data, size_t data_len) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_POST;
  wifi_cmd.resp.size = data_len;
  if (data_len > 0) {
    memcpy(wifi_cmd.resp.bytes, data, data_len);
  }

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

ControllerWiFiResponse ControllerWiFiCheckRequest(void) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CHECK;

  // naming here doesn't really make sense since resp has a resp field
  WiFiCommand resp = WiFiCommand_init_zero;

  WiFiCommandTransaction(&wifi_cmd, &resp);

  // copy the response
  ControllerWiFiResponse http_resp = {};
  http_resp.http_code = resp.rc;
  http_resp.size = resp.resp.size;
  if (http_resp.size > 0) {
    memcpy(http_resp.bytes, resp.resp.bytes, http_resp.size);
  }

  // return timestamp
  return http_resp;
}

bool ControllerWiFiHost(const char *ssid, const char *passwd) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_HOST;

  strncpy(wifi_cmd.ssid, ssid, sizeof(wifi_cmd.ssid));
  strncpy(wifi_cmd.passwd, passwd, sizeof(wifi_cmd.passwd));

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    // ip = NULL;
    return false;
  }

  return true;
}

bool ControllerWiFiStopHost(void) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_STOP_HOST;

  WiFiCommand resp = WiFiCommand_init_zero;

  if (WiFiCommandTransaction(&wifi_cmd, &resp) != CONTROLLER_SUCCESS) {
    return false;
  }

  return true;
}

void ControllerWiFiHostInfo(char *ssid, char *ip, char *mac, uint8_t *clients) {
  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_HOST_INFO;

  WiFiCommand resp = WiFiCommand_init_zero;

  WiFiCommandTransaction(&wifi_cmd, &resp);

  if (ssid != NULL) {
    strcpy(ssid, resp.ssid);
  }

  if (ip != NULL) {
    strcpy(ip, resp.url);
  }

  if (mac != NULL) {
    strcpy(mac, resp.mac);
  }

  if (clients != NULL) {
    *clients = resp.clients;
  }
}
