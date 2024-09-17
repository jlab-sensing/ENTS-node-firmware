#ifndef STM32_LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
#define STM32_LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize WiFi settings on the esp32
 * 
 * The following happens within a single i2c transaction. WiFi SSID, password,
 * endpoint url, endpoint port are transmitted to the esp32. Success or failure
 * is determined by the return code. The timestamp from a NTP server is
 * obtained for clock syncronization.
 * 
 * The function blocks until WiFi is connected and retries until success.
 * 
 * @param ssid WiFi SSID
 * @param passwd WiFi Password
 * @param url API endpoint url (limited to 256 characters)
 * @param port API endpoint port
 * 
 * @return Timestamp indicated by the NTP server
 */
uint32_t ControllerWiFiInit(
  const char* ssid,
  const char* passwd,
  const char* url,
  const char* port
);

/**
 * @brief Post data to the configured endpoint
 * 
 * @param data Binary data
 * @param data_len Length of @p data
 * @param resp Buffer to store response
 * @param resp_len Pointer to store length of @p resp
 * 
 * @return HTTP response code
 */
int ControllerWiFiPost(
  const uint8_t* data,
  size_t data_len,
  uint8_t* resp,
  uint8_t* resp_len
);

#ifdef __cplusplus
}
#endif

#endif // STM32_LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_