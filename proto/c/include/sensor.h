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
  /** Indicate the sensor data needs to be re-uploaded. */
  SENSOR_REUPLOAD,
  /** Indicate an issue with the sensor format. */
  SENSOR_FORMAT
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
SensorStatus EncodeUint32Measurement(Metadata meta, uint32_t value, SensorType type,
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
SensorStatus EncodeInt32Measurement(Metadata meta, int32_t value, SensorType type,
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
 * @brief Gets the size of multiple repeated sensor measurements when encoded.
 *
 * @param meta Metadata for the measurements.
 * @param meas Array of SensorMeasurements to encode.
 * @param count Number of measurements in the array.
 * @param size Pointer to the size variable to populate.
 * 
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus RepeatedSensorMeasurementsSize(Metadata meta, const SensorMeasurement meas[], 
        size_t count, size_t *size);


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
 * @brief Encodes multiple sensor responses into a buffer.
 *
 * This function is mainly here for test purposes as only the server will be
 * sending responses back to the device.
 *
 * @param responses Array of SensorResponses to encode.
 * @param count Number of responses in the array.
 * @param buffer Pointer to the output buffer.
 * @param size Pointer to the size of the output buffer. On success, updated to
 * the number of bytes written.
 *
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus EncodeRepeatedSensorResponses(const RepeatedSensorResponses responses,
    size_t count, uint8_t* buffer, size_t* size);



/**
 * @brief Decodes multiple sensor responses from a buffer.
 *
 * @param data Pointer to the input buffer.
 * @param len Length of the input buffer.
 * @param meas Pointer to the RepeatedSensorResponses to populate.
 *
 * @return SENSOR_SUCCESS on success, SENSOR_ERROR on failure.
 */
SensorStatus DecodeRepeatedSensorReponses(const uint8_t* data, const size_t len,
                             RepeatedSensorResponses* resp);


/**
 * @brief Checks a single sensor response for errors.
 *
 * If the index of the response is 0 and error is not OK, this indicates an
 * overall error in protobuf for the repeated measurement message. The upload
 * should be retried. In this case, SENSOR_REUPLOAD is returned.
 *
 * Otherwise, if there is an issue with the measurement information, ie. a cell
 * or logger is not found or a measurement is not supported. The measurement
 * should be discarded. A SENSOR_FORMAT is returned in this case.
 *
 * Other errors that could possibly result from handling a data buffer or in
 * the communication link should be re-uploaded. In this case, SENSOR_REUPLOAD
 * is returned.
 *
 * @param resp Pointer to the SensorResponse to check.
 *
 * @return SENSOR_OK if no error, SENSOR_ERROR otherwise.
 */
SensorStatus CheckSensorResponse(const SensorResponse* resp);


/**
 * @}
 */


#ifdef __cplusplus
}
#endif

#endif  // PROTO_C_INCLUDE_SENSOR_H_
