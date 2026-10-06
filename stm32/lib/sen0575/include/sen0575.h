/**
 ******************************************************************************
 * @file     sen0575.h
 * @author   Eric Tran
 * @brief    This file contains all the function prototypes for
 *           the sen0575.c file.
 *
 *           This library is designed to read measurements from a SEN0575
 *  rainfall sensor
 * @date     10/5/2026
 ******************************************************************************
 */

#ifndef _SEN0575_H
#define _Sen0575_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ads.h"

// defines
  #define SEN0575_DEVICE_ADDRESS 0x1D

  // SEN0575 stores the address of the input register for PID in memory.
  #define SEN0575_INPUT_REG_PID 0x0000
  // The address of the input register for VID in memory.
  #define SEN0575_INPUT_REG_VID 0x0001
  // The address of the input register for device address in memory.
  #define SEN0575_INPUT_REG_ADDR 0x0002
  // The address of the input register for device baudrate in memory.
  #define SEN0575_INPUT_REG_BAUD 0x0003
  // The address of the input register for RS485 parity bit and stop bit in memory.
  #define SEN0575_INPUT_REG_VERIFYANDSTOP 0x0004
  // The address of the input register for firmware version in memory.
  #define SEN0575_INPUT_REG_VERSION 0x0005
  
  // The address of the input register for low 16-bit cumulative rainfall in set time.
  #define SEN0575_INPUT_REG_TIME_RAIN_FALL_L 0x0006
  // The address of the input register for high 16-bit cumulative rainfall in set time.
  #define SEN0575_INPUT_REG_TIME_RAIN_FALL_H 0x0007
  // The address of the input register for low 16-bit cumulative rainfall since working started.
  #define SEN0575_INPUT_REG_CUMULATIVE_RAINFALL_L 0x0008  
  // The address of the input register for high 16-bit cumulative rainfall since working started.
  #define SEN0575_INPUT_REG_CUMULATIVE_RAINFALL_H 0x0009  
  // The address of the input register for raw data (low 16-bit) in memory.
  #define SEN0575_INPUT_REG_RAW_DATA_L 0x000A  
  // The address of the input register for raw data (high 16-bit) in memory.
  #define SEN0575_INPUT_REG_RAW_DATA_H 0x000B  
  // The address of the input register for system working time in memory.
  #define SEN0575_INPUT_REG_SYS_TIME 0x000C
  // Set the time to calculate cumulative rainfall.
  #define SEN0575_HOLDING_REG_RAW_RAIN_HOUR 0x0006
  // Set the base rainfall value.
  #define SEN0575_HOLDING_REG_RAW_BASE_RAINFALL 0x0007
  
  // The address of the input register for PID in memory.
  #define SEN0575_I2C_REG_PID 0x00
  // The address of the input register for VID in memory.
  #define SEN0575_I2C_REG_VID 0x02
  // The address of the input register for firmware version in memory.
  #define SEN0575_I2C_REG_VERSION 0x0A
  // SEN0575 Stores the cumulative rainfall within the set time
  #define SEN0575_I2C_REG_TIME_RAINFALL 0x0C
  // SEN0575 Stores the cumulative rainfall since starting work
  #define SEN0575_I2C_REG_CUMULATIVE_RAINFALL 0x10
  // SEN0575 Stores the low 16 bits of the raw data
  #define SEN0575_I2C_REG_RAW_DATA 0x14
  // SEN0575 Stores the system time
  #define SEN0575_I2C_REG_SYS_TIME 0x18
  
  // Sets the time for calculating the cumulative rainfall
  #define SEN0575_I2C_REG_RAW_RAIN_HOUR 0x26
  // Sets the base value of accumulated rainfall
  #define SEN0575_I2C_REG_RAW_BASE_RAINFALL 0x28

/**
 ******************************************************************************
 * @brief    Sensor Initialization.
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
HAL_StatusTypeDef sen0575Init(void);

/**
 ******************************************************************************
 * @brief    Returns the uptime of the sensor
 *
 * @param    void
 * @return   time
 ******************************************************************************
 */
// TODO: create a measurement type in the protobuff for sen0575
SEN0257Measurement sen0575WorkingTime(void);

/**
 ******************************************************************************
 * @brief    Returns cumulative rainfall
 *
 * @param    void
 * @return   measurements
 ******************************************************************************
 */
SEN0257Measurement sen0575RainfallCumulative(void);

/**
 ******************************************************************************
 * @brief    Count of the number of times the tipping bucket has tipped
 *
 * @param    void
 * @return   bucket tip count
 ******************************************************************************
 */
uint32_t sen0575RainfallRaw(void);



/**
 * @brief Read water pressure sensor and serialize measurement
 *
 * The voltage output of the water pressure is measured. A calibration is
 * applied to convert voltage into a leaf wetness measurement.
 *
 * Current voltage and pressure are the same value, until a calibration
 * is obtained.
 *
 *
 * @see SensorsPrototypeMeasure
 */
size_t WatPress_measure(uint8_t *data, SysTime_t ts, uint32_t idx);

#ifdef __cplusplus
}
#endif

#endif  _SEN0575_H