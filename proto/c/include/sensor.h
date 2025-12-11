/**
 * @file sensor.h
 *
 * @brief Library for sensor measurement encoding/decoding
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2025-12-09
 */

#ifndef PROTO_C_INCLUDE_SENSOR_H_
#define PROTO_C_INCLUDE_SENSOR_H_

#include <stdint.h>
#include <stddef.h>

#include "sensor.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup proto
 * @defgroup sensor Sensor Measurement Serialization
 * @brief Library for encoding and decoding sensor measurements.
 *
 * Where sensors measurements are taken EncodeUint64Measurement,
 * EncodeInt64Measurement, EncodeDoubleMeasurement, and EncodeTextMeasurement
 * are used.
 *
 * When formatting multiple measurements, single measurements may have to be
 * decoded first with DecodeSensorMeasurement and placed in the SensorMeasurement struct before re-encoding
 * Re-encoding into a RepeatedSensorMeasurement struct can be done with
 * EncodeRepeatedSensorMeasurements.
 *
 * @{
 */

typedef enum _SensorStatus {
  SENSOR_OK,
  SENSOR_ERROR,
  SENSOR_OUT_OF_BOUNDS,
} SensorStatus;

/** Constant value to indicate no metadata field */
static const Metadata METADATA_NONE = Metadata_init_zero;

/**
 * @brief Encodes a single sensor measurement into a buffer.
 *
 * The metadata is embedded within the SensorMeasurement structure.
 *
 * @param meas Pointer to the SensorMeasurement to encode.
 * @param buffer Pointer to the output buffer.
 * @param size Pointer to the size of the output buffer. On success, updated to
 * the number of bytes written.
 *
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus EncodeSensorMeasurement(const SensorMeasurement* meas, 
    uint8_t* buffer, size_t* size);

/**
 * @brief Encodes multiple sensor measurements into a buffer.
 *
 * No checking is done to ensure there are not duplicates in the metadata. This
 * is left up to the user implementation.
 *
 * @param meta Metadata for the measurements.
 * @param meas Array of SensorMeasurements to encode.
 * @param count Number of measurements in the array.
 * @param buffer Pointer to the output buffer.
 * @param size Pointer to the size of the output buffer. On success, updated to
 * the number of bytes written.
 *
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus EncodeRepeatedSensorMeasurements(Metadata meta, const SensorMeasurement meas[],
    size_t count, uint8_t* buffer, size_t* size);

/**
 * @brief Encodes a uint64_t sensor measurement into a buffer.
 *
 * @param meta Metadata for the measurement.
 * @param value The uint64_t measurement value.
 * @param type The SensorType of the measurement.
 * @param buffer Pointer to the output buffer.
 * @param size Pointer to the size of the output buffer. On success, updated to
 * the number of bytes written.
 * 
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus EncodeUint64Measurement(Metadata meta, uint64_t value, SensorType type,
                             uint8_t* buffer, size_t* size);

/**
 * @brief Encodes an int64_t sensor measurement into a buffer.
 *
 * @param meta Metadata for the measurement.
 * @param value The int64_t measurement value.
 * @param type The SensorType of the measurement.
 * @param buffer Pointer to the output buffer.
 * @param size Pointer to the size of the output buffer. On success, updated to
 * the number of bytes written.
 * 
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus EncodeInt64Measurement(Metadata meta, int64_t value, SensorType type,
                            uint8_t* buffer, size_t* size);

/**
 * @brief Encodes a double sensor measurement into a buffer.
 *
 * @param meta Metadata for the measurement.
 * @param value The double measurement value.
 * @param type The SensorType of the measurement.
 * @param buffer Pointer to the output buffer.
 * @param size Pointer to the size of the output buffer. On success, updated to
 * the number of bytes written.
 * 
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus EncodeDoubleMeasurement(Metadata meta,
                             double value, SensorType type,
                             uint8_t* buffer, size_t* size);

/**
 * @brief Decodes a sensor measurement from a buffer.
 *
 * @param data Pointer to the input buffer.
 * @param len Length of the input buffer.
 * @param meas Pointer to the SensorMeasurement to populate.
 * 
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus DecodeSensorMeasurement(const uint8_t* data, const size_t len,
                             SensorMeasurement* meas);

/**
 * @brief Decodes multiple sensor measurements from a buffer.
 *
 * @param data Pointer to the input buffer.
 * @param len Length of the input buffer.
 * @param meas Pointer to the RepeatedSensorMeasurements to populate.
 * 
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus DecodeRepeatedSensorMeasurements(const uint8_t* data, const size_t len,
                             RepeatedSensorMeasurements* meas);

/**
 * @}
 */

#endif  // PROTO_C_INCLUDE_SENSOR_H_
