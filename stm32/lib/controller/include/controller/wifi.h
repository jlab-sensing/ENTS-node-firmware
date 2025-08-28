/**
 * @file wifi.h
 * @date 2024-10-13
 * @author John Madden <jmadden173@pm.me>
 * @brief WiFi interface implementation with the esp32
 */

#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_

#include <stdbool.h>
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
 * @brief WiFi status
 *
 * The enum is copied from the Arduino WiFi library. See the following for the
 * definitions:
 * https://github.com/arduino-libraries/WiFi/blob/e0e655793d73bfe5c8616bf753c3662dcdf98ad9/src/utility/wl_definitions.h#L59
 *
 * The meaning of these can be ambiguous. See the following forum for the
 * sequence of status during connect and disconnect events:
 * https://forum.arduino.cc/t/serial-println-wifi-status-codes-meaning/486265/5
 */
typedef enum {
  CONTROLLER_WIFI_NO_SHIELD = 255,
  CONTROLLER_WIFI_IDLE_STATUS = 0,
  CONTROLLER_WIFI_NO_SSID_AVAIL,
  CONTROLLER_WIFI_SCAN_COMPLETED,
  CONTROLLER_WIFI_CONNECTED,
  CONTROLLER_WIFI_CONNECT_FAILED,
  CONTROLLER_WIFI_CONNECTION_LOST,
  CONTROLLER_WIFI_DISCONNECTED
} ControllerWiFiStatus;

/**
 * @brief WiFi response
 *
 * The size of bytes is set to a reasonable value of 256 bytes as reponses need
 * to be under 222 bytes for LoRaWAN. Set independently from the protobuf
 * implementation as the API check may require a larger reponse in the future.
 */
typedef struct {
  /** HTTP Code */
  unsigned int http_code;
  /** Response bytes */
  uint8_t bytes[256];
  /** Size of bytes */
  size_t size;
} ControllerWiFiResponse;

/**
 * @brief Initialize WiFi settings on the esp32
 *
 * Connect to a WiFi network with the given SSID and password. This function
 * should be used in a polling configuration to continually check if the esp32
 * has connected to the WiFi network. The first call initializes the connection
 * and subsequent calls check the connection status.
 *
 * @param ssid WiFi SSID
 * @param passwd WiFi Password
 *
 * @return If the command succeeded
 */
bool ControllerWiFiConnect(const char *ssid, const char *passwd);

/**
 * @brief Disconnects from the WiFi network
 *
 * Returns immediately after signal is sent to the esp32 to disconnect from the
 * current WiFi network. @see ControllerWiFiCheckWiFi to check when the action
 * has completed.
 *
 * @return If the command succeeded
 */
bool ControllerWiFiDisconnect(void);

/**
 * @brief Checks the current status of WiFi
 *
 * @return WiFi status code
 */
ControllerWiFiStatus ControllerWiFiCheckWiFi(void);

/**
 * @brief Triggers a NTP sync with the server
 *
 * @see ControllerWiFiTime to get the current time.
 *
 * @return If the command succeeded
 */
bool ControllerWiFiNtpSync(void);

/**
 * @brief Get the current time from the NTP server
 *
 * Errors are indicated by a 0 return value.
 *
 * @return Timestamp indicated by the NTP server
 */
uint32_t ControllerWiFiTime(void);

/**
 * @brief Queries the API endpoint to check if it is reachable
 *
 * Get the http code and response with @see ControllerWiFiCheckRequest.
 *
 * @param url API endpoint url (limited to 256 characters)
 * @param port API endpoint port
 *
 * @return If the command succeeded
 */
bool ControllerWiFiCheckApi(const char *url);

/**
 * @brief Post data to the configured endpoint
 *
 * Get the http code and response with @see ControllerWiFiCheckRequest.
 *
 * @param data Binary data
 * @param data_len Length of @p data
 * @param resp Buffer to store response
 * @param resp_len Pointer to store length of @p resp
 *
 * @return If the command succeeded
 */
bool ControllerWiFiPost(const uint8_t *data, size_t data_len);

/**
 * @brief Checks the status of the most recent HTTP request
 *
 * A http_code of 0 indicates that the request was not sent and possibly an
 * error with the WiFi connection.
 *
 * This function MUST be called immediately after a HTTP request command.
 * Currently unverified how arduino code handles http requests but could cause
 * issues with the server timing out before the esp32 reads the bytes.
 *
 * @return HTTP response code and response data
 */
ControllerWiFiResponse ControllerWiFiCheckRequest(void);

/**
 * @brief Sets the esp32 as a WiFi access point
 *
 * This function configures the esp32 to act as a WiFi access point with the
 * given SSID and password. If the password is an empty string, the access point
 * will be open (no password).
 *
 * @param ssid SSID for the access point
 * @param passwd Password for the access point (empty for open)
 *
 * @return If the command succeeded
 */
bool ControllerWiFiHost(const char *ssid, const char *passwd);

/**
 * @brief Stops the esp32 from acting as a WiFi access point.
 *
 * Does not turn off the WiFi module on the esp32.
 *
 * @return If the command succeeded
 */
bool ControllerWiFiStopHost(void);

/**
 * @brief Gets information about the current access point.
 *
 * Copies the SSID, IP address, and MAC address of the access point into
 * the provided buffers. The buffers must be large enough to hold the
 * respective strings. SSID should be at least 32 bytes, IP should be at least
 * 16 bytes, and MAC should be at least 18 bytes.
 */
void ControllerWiFiHostInfo(char *ssid, char *ip, char *mac);
/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
