/**
 * @file teros21.h
 * @author John Madden <jmadden173@pm.me>
 * @brief Drivers for reading measurements from Teros21 sensor
 * @date 2025-03-05
 */

#ifndef LIB_SDI12_INCLUDE_TEROS21_H_
#define LIB_SDI12_INCLUDE_TEROS21_H_

#include "sdi12.h"
#include "stm32_systime.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup sdi12
 * @defgroup teros21 Teros21
 * @brief Library for interfacing with Teros21 sensors
 *
 * This library is designed to read measurements from Teros21 sensors.
 *
 * Datasheet: https://metergroup.com/products/teros-21/
 *
 * Examples:
 * - @ref example_teros21.c
 * - @ref example_rawTeros21.c
 *
 * @{
 */

typedef struct {
  char addr;
  float matric_pot;
  float temp;
} Teros21Data;

/**
 * @brief Parse measurement string from Teros21 sensor
 */
SDI12Status Teros21ParseMeasurement(const char *buffer, Teros21Data *data);

/**
 * @brief Read and parse a Teros21 measurement
 *
 * @param addr Address of the sensor
 * @param data Pointer to the data structure to store the measurement
 * @return SDI12Status
 */
SDI12Status Teros21GetMeasurement(char addr, Teros21Data *data);

/**
 * @brief Get measurement from Teros21 sensor
 *
 * Measures a Teros21 at address 0 and encodes it into a serialized
 * measurement.
 *
 * @param data Buffer to store measurement
 * @param ts Timestamp of measurement
 * @param idx Index of measurement
 * @return Length of measurement
 *
 * @see SensorsPrototypeMeasure
 *
 */
size_t Teros21Measure(uint8_t *data, SysTime_t ts, uint32_t idx);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_SDI12_INCLUDE_TEROS21_H_
