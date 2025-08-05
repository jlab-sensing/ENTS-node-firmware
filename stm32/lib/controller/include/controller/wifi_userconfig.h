#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_

#include <stddef.h>
#include <stdint.h>

#include "communication.h"
#include "usart.h"
#include "userConfig.h"

#ifdef __cplusplus
extern "C" {
#endif
extern char uart_buf[512];

/**
 * @brief Initialize WiFi settings on the esp32
 *
 * Connect to a WiFi network with the given SSID and password. This function
 * should be used in a polling configuration to continually check if the esp32
 * has connected to the WiFi network. The first call initializes the connection
 * and subsequent calls check the connection status.
 *
 * The return code is from Arduino WiFi.begin() with available at the following
 * link:
 * https://github.com/arduino-libraries/WiFi/blob/e0e655793d73bfe5c8616bf753c3662dcdf98ad9/src/utility/wl_definitions.h#L59
 *
 * @param ssid WiFi SSID
 * @param passwd WiFi Password
 *
 * @return WiFi status code
 */
uint8_t ControllerWiFiInit(const char *ssid, const char *passwd);

/**
 * @brief Get the current time from the NTP server
 *
 * @return Timestamp indicated by the NTP server
 */
uint32_t ControllerWiFiTime(void);

/**
 * @brief Checks WiFi connection status by querying API status
 *
 * @param url API endpoint url (limited to 256 characters)
 * @param port API endpoint port
 *
 * @return HTTP response code
 */
unsigned int ControllerWiFiCheck(const char *url, const uint32_t port);

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
int ControllerWiFiPost(const uint8_t *data, size_t data_len, uint8_t *resp,
                       uint8_t *resp_len);

/**
 * @brief Request user configuration from ESP32
 *
 * @return UserConfigStatus indicating success or failure
 */
UserConfigStatus ControllerUserConfigRequest(void);

/**
 * @brief Send user configuration to ESP32
 *
 * @return UserConfigStatus indicating success or failure
 */
UserConfigStatus ControllerUserConfigSend(void);

/**
 * @brief Check if configuration is empty/uninitialized
 *
 * @param config Pointer to UserConfiguration structure
 * @return true if config is empty, false otherwise
 */
bool isConfigEmpty(const UserConfiguration *config);

/**
 * @brief Print user configuration details to log
 *
 * @param config Pointer to UserConfiguration structure
 */
void printUserConfig(const UserConfiguration *config);

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_