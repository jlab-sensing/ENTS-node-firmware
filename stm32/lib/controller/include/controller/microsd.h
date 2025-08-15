/**
 * @file microsd.h
 * @date 2025-07-30
 * @author Jack Lin <jlin143@ucsc.edu>
 * @brief MicroSD interface implementation with the esp32
 */

#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_MICROSD_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_MICROSD_H_

#include <stddef.h>
#include <stdint.h>

#include "userConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup controller
 * @defgroup controllerMicroSD MicroSD
 * @brief MicroSD interface for the esp32
 *
 *
 * The *MicroSDCommand* message is used to control the microSD card connected
 * to the esp32 module's SPI.
 *
 * @{
 */

/**
 * @brief Save data to the microSD card. Appends data to already-created files,
 * otherwise creates a new file.
 *
 * @param data An array of data bytes. This data must be a protobuf-encoded
 * (serialized) Measurement.
 * @param num_bytes The number of bytes to be written.
 *
 * @return File length in bytes (after saving data). Returns -1 on failure to
 * save.
 */
uint32_t ControllerMicroSDSave(const uint8_t *data, const uint16_t num_bytes);

/**
 * @brief Send the UserConfig to the ESP32 and saves a file on the microSD with
 * the UserConfig contents.
 *
 * @param uc UserConfig to be encoded and sent.
 * @param filename Filename (limited to 256 characters)
 *
 * @return File length in bytes (after saving data). Returns -1 on failure to
 * save.
 */
uint32_t ControllerMicroSDUserConfig(UserConfiguration *uc,
                                     const char *filename);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_MICROSD_H_
