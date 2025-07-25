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

#ifndef PROTO_C_INCLUDE_TRANSCODER_H_
#define PROTO_C_INCLUDE_TRANSCODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <string.h>

#include "soil_power_sensor.pb.h"

/**
 * @ingroup proto
 * @defgroup protoTranscoder Transcoder
 * @brief Library for encoding/decoding protobuf messages
 *
 * @{
 */

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
size_t EncodePowerMeasurement(uint32_t ts, uint32_t logger_id, uint32_t cell_id,
                              double voltage, double current, uint8_t *buffer);

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
 * @brief Encodes a BME280 measurement
 *
 * The timestamp is not able to encode timezones and is references from UTC+0.
 * The serialized data is stored in @p buffer with the number of bytes written
 * being returned by the function. A return value of -1 indicates an error in
 * encoding.
 *
 * Below shows the relation between raw measurements taken from the device in
 * relation to real-world or si units.
 *
 * Raw values
 *
 * pressure: 98473
 * temperature: 2275
 * humidity: 43600
 *
 * SI unit values
 *
 * pressure: 9847.3 hPa
 * temperature: 22.75 C
 * humidity: 43.600 %
 *
 * @param ts Timestamp
 * @param logger_id Logger Id
 * @param cell_id Cell Id
 * @param pressure Air pressure in hPa
 * @param temperature Air temperature in celsius (C)
 * @param humidity Relative humidity in percent (%)
 * @param buffer Buffer to store serialized measurement
 * @return Number of bytes in @p buffer
 */
size_t EncodeBME280Measurement(uint32_t ts, uint32_t logger_id,
                               uint32_t cell_id, uint32_t pressure,
                               int32_t temperature, uint32_t humidity,
                               uint8_t *buffer);

                               
/**
 * @brief Encodes a Water Pressure measurement
 *
 * Currently only the voltage measurement is used. Sensor will
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
 * @param water_pressure Water Pressure
 * @param buffer Buffer to store serialized measurement
 * @return Number of bytes in @p buffer
 */
size_t EncodeWaterPressMeasurement(uint32_t ts, uint32_t logger_id,
    uint32_t cell_id, double voltage,
    double water_pressure, uint8_t *buffer);

/**
 * @brief Encodes a Capacitive Soil Moisture measurement
 *
 * Currently only the voltage measurement is used. Sensor will
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
 * @param soil_moister Calibrated soil moister
 * @param buffer Buffer to store serialized measurement
 * @return Number of bytes in @p buffer
 */
size_t EncodeSEN0308Measurement(uint32_t ts, uint32_t logger_id,
    uint32_t cell_id, double voltage,
    double humidity, uint8_t *buffer);

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
                         size_t n, uint8_t *buffer, size_t size);

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
                         uint8_t *buffer, size_t size);

/**
 * @brief Encodes a WiFiCommand
 *
 * @param wifi_cmd Command containing the data
 * @param buffer Buffer to store serialized measurement
 * @param size Size of buffer
 *
 * @return Number of bytes in @p buffer
 */
size_t EncodeWiFiCommand(const WiFiCommand *wifi_cmd, uint8_t *buffer,
                         size_t size);

/**
 * @brief Encodes user configuration data.
 *
 * This function serializes user configuration settings, including upload
 * settings, measurement settings, and WiFi settings, using protobuf and stores
 * the result in a buffer. The serialized data length is returned, or -1 if an
 * encoding error occurred.
 *
 * @param config User configuration to encode
 * @param buffer Buffer to store serialized data
 * @return Number of bytes in the buffer, or -1 on error
 */
size_t EncodeUserConfiguration(UserConfiguration *config, uint8_t *buffer);

/**
 * @brief Decodes user configuration data.
 *
 * This function deserializes a user configuration protobuf message from a
 * buffer. The message length is specified, and a return value of -1 indicates a
 * decoding error.
 *
 * @param data Serialized data buffer
 * @param len Length of data buffer
 * @param config Decoded user configuration output
 * @return 0 on success, -1 on error
 */
int DecodeUserConfiguration(const uint8_t *data, const size_t len,
                            UserConfiguration *config);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // PROTO_C_INCLUDE_TRANSCODER_H_
