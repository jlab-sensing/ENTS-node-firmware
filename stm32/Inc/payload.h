/**
 * @file payload.h
 *
 * @brief Payload handling.
 *
 * Serves as the interface between the FIFO buffer and the upload method.
 * Allows repeated sensor measurements to be packed into the maximum payload
 * size allowed.
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2026-01-02
 */

#ifndef STM32_INC_PAYLOAD_H_
#define STM32_INC_PAYLOAD_H_


#include <stdint.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif



typedef enum _PayloadStatus {
  PAYLOAD_OK,
  PAYLOAD_ERROR,
  PAYLOAD_NO_DATA,
} PayloadStatus;



/**
 * @brief Formats the next payload from the FIFO buffer.
 *
 * Reads measurements from the FIFO buffer and packs them into the provided
 * buffer as a RepeatedSensorMeasurements protobuf message. The number of bytes
 * written is returned in size.
 *
 * @param buffer Pointer to the output buffer.
 * @param size Size of the output buffer.
 * @param length Pointer to the length of the formatted payload.
 *
 * @return PAYLOAD_OK on success, PAYLOAD_NO_DATA if no data is available,
 * PAYLOAD_ERROR on failure.
 */
PayloadStatus FormatPayload(uint8_t* buffer, size_t size, size_t* length);


#ifdef __cplusplus
}
#endif

#endif  // STM32_INC_PAYLOAD_H_
