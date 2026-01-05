/**
 * @file teros12.h
 * @author John Madden <jmadden173@pm.me>
 * @brief Drivers for reading measurements from Teros12 sensor
 * @date 2025-03-05
 */

#ifndef LIB_SDI12_INCLUDE_TEROS12_H_
#define LIB_SDI12_INCLUDE_TEROS12_H_

#include "sdi12.h"
#include "stm32_systime.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup sdi12
 * @defgroup teros12 Teros12
 * @brief Library for interfacing with Teros12 sensors
 *
 * This library is designed to read measurements from Teros12 sensors.
 *
 * Datasheet: https://metergroup.com/products/teros-12/
 *
 * Examples:
 * - @ref example_teros12.c
 *
 * @{
 */

typedef struct {
  char addr;
  float vwc;
  float temp;
  unsigned int ec;
} Teros12Data;

/**
 * @brief Parse measurement string from Teros21 sensor
 *
 * The buffer is expected to be the following format. [+-] can either be a + or
 * - sign.:
 * a+<calibratedCountsVWC>[+-]<temperature>+<electricalConductivity>
 *
 * Example real world values:
 * 0+1846.16+22.3+1
 *
 * @param buffer Raw measurement string
 * @param data Pointer to the data structure to store the measurement
 * @return SDI12Status
 */
SDI12Status Teros12ParseMeasurement(const char *buffer, Teros12Data *data);

/**
 * @brief Read and parse a Teros12 measurement
 *
 * @param addr Address of the sensor
 * @param data Pointer to the data structure to store the measurement
 * @return SDI12Status
 */
SDI12Status Teros12GetMeasurement(char addr, Teros12Data *data);

/**
 * @brief Get measurement from Teros12 sensor
 *
 * Measures a Teros12 at address 0 and encodes it into a serialized
 * measurement.
 *
 * @param data Buffer to store measurement
 * @return Length of measurement
 *
 * @see SensorsPrototypeMeasure
 *
 */
size_t Teros12Measure(uint8_t *data, SysTime_t ts, uint32_t idx);

/**
 * @}
 */

#endif  // LIB_SDI12_INCLUDE_TEROS12_H_
