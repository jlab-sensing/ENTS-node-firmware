/**
 ******************************************************************************
 * @file     as7343.h
 * @author   Eric Tran
 * @brief    This file contains all the function prototypes for
 *           the as7343.c file.
 *
 *           This library is designed to read measurements from a Multi-Spectral
 * Sensor
 *           https://github.com/sparkfun/SparkFun_AS7343_Arduino_Library/blob/main/examples/Example_01_BasicReadings/Example_01_BasicReadings.ino
 * @date     7/3/2026
 ******************************************************************************
 */

#ifndef _AS7343_H
#define _AS7343_H

#include <stddef.h>
#include <stdint.h>

#include "as7343_defs.h"
#include "i2c.h"
#include "sensors.h"
#include "stm32_systime.h"

// typedef struct {
//   uint16_t channelLow[ksfAS7343NumChannels];
//   uint16_t channelHigh[ksfAS7343NumChannels];
//   uint16_t channelCombined[ksfAS7343NumChannels];
// } AS7343Data;

typedef union{
  // uint8_t halfChannel[ksfAS7343NumChannels * 2]; // HAL_I2C commands use 8 bits
  uint16_t channelCombined[ksfAS7343NumChannels]; // used to access full channeldata
} AS7343Data;


/*! CPP guard */
#ifdef __cplusplus
extern "C" {
#endif

/**
******************************************************************************
* @brief    Initializes the AS7343 sensor.
*
* @param    void
* @return   SUCCESS or ERROR
******************************************************************************
*/
int8_t AS7343Init(void);

/**
******************************************************************************
* @brief    Collects Spectral Data from 14 Channels
*
* Records the ongoing spectral readings measured by the sensor into ChannelData
* to allow for further parsing to occur
*
* @param    channelData Spectral Data
* @return   AS7343Data
******************************************************************************
*/
void AS7343GetMeasurement(AS7343Data *channelData);

/**
******************************************************************************
* @brief    Enables spectral data readings
*
* Allows the sensor to transition from a power saving idle
* state to an active one where it will be able to collect measurements
*
* @param    void
* @return   void
******************************************************************************
*/
void AS7343Active(void);

/**
******************************************************************************
* @brief    Disables Sthe active collection of spectral data to enter power
* saving mode
*
* Allows the sensor to transition from an active state state
* to a power saving state where it is unable to collect measurements
*
* @param    void
* @return   void
******************************************************************************
*/
void AS7343Idle(void);

/**
******************************************************************************
* @brief    Turns on LED
*
* Led is turned on for better readings in low light environments
*
* @param    void
* @return   void
******************************************************************************
*/
void AS7343LEDOn(void);

/**
******************************************************************************
* @brief    Turns off LED
*
* @param    void
* @return   void
******************************************************************************
*/
void AS7343LEDOff(void);

/**
 * @brief Read spectral sensor and serialize measurement
 *
 * The voltage output of the spectral sensor is measured. A calibration
 * is then applied. Data gets encoded into a serialized measurement.
 *
 * @param data Buffer to store measurement
 * @param ts System time of measurement
 * @param idx Sensor index
 * @return Length of measurement
 *
 * @see SensorsPrototypeMeasure
 */
size_t AS7343Measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                     EnabledSensorMultiple *sensor);

// TODO: Add function to print AS7343Data in a human readable format (take note
// of )

#ifdef __cplusplus
}
#endif

#endif  // _AS7343_H
