/**
 * @file wifi.h
 * @date 2024-10-13
 * @author John Madden <jmadden173@pm.me>
 * @brief WiFi interface implementation with the esp32
 */

#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup controller
 * @defgroup controllerWiFi WiFi
 * @brief WiFi interface for the esp32
 *
 *
 * The *WiFiCommand* message is used to control the WiFi hardware on the esp32
 * module. There are two types: (1) `CONNECT` which is used to connect to the
 * WiFi network and (2) `POST` which sends data to an API endpoint.

 * @image html WiFiCommand.png "WiFiCommand Message Structure" width=60%
 *
 * The return code (`rc`) message serves dual purpose based on the type and
 * direction of communication. For the `CONNECT`, the return code is used to
 * send a WiFi enum directly from Arduino `WiFi.status()`. See [this
 * file](https://github.com/arduino/wifishield/blob/0d0b242c794e3e91126fd72dbcca906829226128/firmware/wifiHD/src/wl_definitions.h#L28)
 * for possible values. For `POST` the return code is used to store the HTTP
 * response code from the *POST* request. See [HTTP status
 * code](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status) for possible
 * values.
 *
 * The software flow for WiFi is as follows:
 *
 * @image html esp32controller.png "Esp32 Controller" width=50%
 *
 * @{
 */

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
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
