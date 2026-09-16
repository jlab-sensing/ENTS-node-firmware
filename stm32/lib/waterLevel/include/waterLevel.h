/**
 ******************************************************************************
 * @file     waterLevel.h
 * @author   Eric Tran
 * @brief    This file contains all the function prototypes for
 *           the waterLevel.c file.
 *
 *           This library is designed to read measurements from a ALS-MPM-2F
 * water level sensor
 * @date     8/14/2026
 ******************************************************************************
 */

#ifndef LIB_WATERLEVEL_INCLUDE_WATERLEVEL_H_
#define LIB_WATERLEVEL_INCLUDE_WATERLEVEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "sensors.h"

#define WATER_LEVEL_HW685_ADAPTER
// #define WATER_LEVEL_RESISTOR_DIVIDER

#if defined(WATER_LEVEL_HW685_ADAPTER) && defined(WATER_LEVEL_RESISTOR_DIVIDER)
#error Enable only one of the water level measurement modes in waterLevel.h.
#endif

// Water level sensors are typically marked with their maximum sensing depth.
#define WATER_LEVEL_SENSOR_MIN_DEPTH_M 0.0
#define WATER_LEVEL_SENSOR_MAX_DEPTH_M 2.0

#ifdef WATER_LEVEL_HW685_ADAPTER

// Set HW-685 jumper 1-2 OPEN, jumper 3-4 SHORT
// Set HW-685 adapter ZERO point 0 mV @ 0 mA and SPAN point 3300 mV @ 20 mA
//  0 mA =    0 mV
//  4 mA =  660 mV
// 12 mA = 1980 mV
// 20 mA = 3300 mV
#define WATER_LEVEL_V_MIN 0.660
#define WATER_LEVEL_V_MAX 3.300

#elif defined(WATER_LEVEL_RESISTOR_DIVIDER)

// Resistor value used to read the current output from the 4-20 mA sensor.
#define WATER_LEVEL_RESISTOR_OHMS 150.0

#define WATER_LEVEL_I_MIN 0.004
#define WATER_LEVEL_I_MAX 0.020

// V_min = 4 mA * 150 Ohm = 600 mV
// V_max = 20 mA * 150 Ohm = 3000 mV
#define WATER_LEVEL_V_MIN (WATER_LEVEL_I_MIN * WATER_LEVEL_RESISTOR_OHMS)
#define WATER_LEVEL_V_MAX (WATER_LEVEL_I_MAX * WATER_LEVEL_RESISTOR_OHMS)

#endif

// Macro for converting voltages in the range [V_min, V_max] to the depth in
// range [WATER_LEVEL_SENSOR_MIN_DEPTH_M, WATER_LEVEL_SENSOR_MAX_DEPTH_M]
// Negative depths indicate I < 4 mA, check sensor connection.
#define WATER_LEVEL_VOLTAGE_TO_METERS(v)                                 \
  (((v - WATER_LEVEL_V_MIN) / (WATER_LEVEL_V_MAX - WATER_LEVEL_V_MIN)) * \
   WATER_LEVEL_SENSOR_MAX_DEPTH_M)

/**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
void WaterLevelInit(EnabledSensorMultiple *sensor);

/**
 ******************************************************************************
 * @brief    Returns both thecalibrated measurement
 *           from a water level sensor.
 *
 * @param    void
 * @return   ALSMPM2FMeasurement
 ******************************************************************************
 */
ALSMPM2FMeasurement WatLevelGetMeasurement(EnabledSensorMultiple *sensor);

/**
 * @brief Read water Level sensor and serialize measurement
 *
 * The voltage output of the water Level is measured. A calibration is
 * applied to convert voltage into a water depth measurement.
 *
 *
 * @see SensorsPrototypeMeasure
 */
size_t WatLevel_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                        EnabledSensorMultiple *sensor);

#ifdef __cplusplus
}
#endif

#endif  // LIB_WATERPRESSURE_INCLUDE_WATERPRESSURE_H_
