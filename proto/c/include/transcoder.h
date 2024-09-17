/**
 * @file transcoder.h
 * 
 * @brief Library for encoding/decoding protobuf messages
 * 
 * Encoding functions are provided for each measurement type. A decoding
 * function is provided for a Response message. If more functionality is
 * required, use the protobuf generated located in soil_power_sensor.pb.h.
 * 
 * @see transcoder.c
 * @see test_transcoder.c
 * 
 * @author John Madden <jmadden173@pm.me>
 * @date 2024-01-04
*/

#ifndef __TRANSCODER_H__
#define __TRANSCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stddef.h>

#include "soil_power_sensor.pb.h"

/**
 * @brief Encodes a power measurement
 * 
 * The timestamp is not able to encode timezones and is references from UTC+0.
 * The serialized data is stored in @p buffer with the number of bytes written
 * being returned by the function. A return value of -1 indicates an error in
 * encoding.
 * 
 * @param ts Unix epochs
 * @param logger_id Logger Id
 * @param cell_id Cell Id
 * @param voltage Voltage measured in mV
 * @param current Current measured in uA
 * @param buffer Buffer to store serialized measurement
 * @return Number of bytes in @p buffer
*/
size_t EncodePowerMeasurement(uint32_t ts, uint32_t logger_id,
                              uint32_t cell_id, double voltage,
                              double current, uint8_t *buffer);

/**
 * @brief Encodes a Teros12 measurement
 * 
 * The timestamp is not able to encode timezones and is references from UTC+0.
 * The serialized data is stored in @p buffer with the number of bytes written
 * being returned by the function. A return value of -1 indicates an error in
 * encoding.
 * 
 * @param ts Timestamp
 * @param logger_id Logger Id
 * @param cell_id Cell Id
 * @param vwc_raw Raw volumetric water content
 * @param vwc_adj Volumetric water content converted to percentage
 * @param temp Temperature in celsius
 * @param ec Electrical conductivity
 * @param buffer Buffer to store serialized measurement
 * @return Number of bytes in @p buffer
*/
size_t EncodeTeros12Measurement(uint32_t ts, uint32_t logger_id,
                                uint32_t cell_id, double vwc_raw,
                                double vwc_adj, double temp, uint32_t ec,
                                uint8_t *buffer);

/**
 * @brief Encodes a Phytos31 measurement
 * 
 * Currently only the voltage measurement is used. Leaf wetness will
 * be implemented once more is known about the sensor.
 * 
 * The timestamp is not able to encode timezones and is references from UTC+0.
 * The serialized data is stored in @p buffer with the number of bytes written
 * being returned by the function. A return value of -1 indicates an error in
 * encoding.
 * 
 * @param ts Timestamp
 * @param logger_id Logger Id
 * @param cell_id Cell Id
 * @param voltage Raw voltage reading
 * @param leaf_wetness Calibrated leaf wetness
 * @param buffer Buffer to store serialized measurement
 * @return Number of bytes in @p buffer
 */
size_t EncodePhytos31Measurement(uint32_t ts, uint32_t logger_id,
                                 uint32_t cell_id, double voltage,
                                 double leaf_wetness, uint8_t *buffer);

/**
 * @brief Decodes a response message
 * 
 * Take bytes in @p data withy length @p len and decodes into a response type.
 * Neither @p data or @p len are modified. A return value of -1 indicates an
 * error in decoding. See soil_power_sensor.proto for a full list of response
 * types.
 * 
 * @param data Protobuf serialized data
 * @param len Number of bytes in @p data
 * @return Response type
*/
Response_ResponseType DecodeResponse(const uint8_t *data, const size_t len);

/**
 * @brief Decodes an Esp32Command message
 * 
 * The type of command and the data from the command will need to be manually
 * extracted, @see soil_power_sensor.ph.h.
 * 
 * @returns Esp32Command data
 */
Esp32Command DecodeEsp32Command(const uint8_t *data, const size_t len);

/**
 * @brief Encodes a page command
 * 
 * @param req Request type
 * @param fd File descriptor
 * @param bs Block size
 * @param n Number of bytes
 * @param buffer Buffer to store serialized command
 * @param size Size of buffer
 * 
 * @returns Number of bytes in @p buffer
 */
size_t EncodePageCommand(PageCommand_RequestType req, int fd, size_t bs,
                         size_t n, uint8_t* buffer, size_t size);

/**
 * @brief Encodes a test command
 * 
 * @param state State to change to
 * @param data Data to encode
 * @param buffer Buffer to store serialized command
 * @param size Size of buffer
 * 
 * @return Number of bytes in @p buffer
 */
size_t EncodeTestCommand(TestCommand_ChangeState state, int32_t data,
                         uint8_t* buffer, size_t size);

/**
 * @brief Encodes a WiFiCommand
 * 
 * If a field is not used, set to its default value to save space.
 * 
 * @param type Command type
 * @param ssid WiFi SSID
 * @param passwd WiFi Password
 * @param url API endpoint url (limited to 256 characters)
 * @param port API endpoint port
 * @param rc Return code
 * @param ts Timestamp
 * @param resp Serialized response
 * @param resp_len Length of @p resp
 * @param buffer Buffer to store serialized measurement
 * @param size Size of buffer
 * 
 * @return Number of bytes in @p buffer
 */
size_t EncodeWiFiCommand(
  WiFiCommand_Type type,
  const char* ssid,
  const char* passwd,
  const char* url,
  uint32_t port,
  uint32_t rc,
  uint32_t ts,
  const uint8_t* resp,
  size_t resp_len,
  uint8_t* buffer,
  size_t size
);

#ifdef __cplusplus
}
#endif

#endif /* __TRANSCODER_H__ */