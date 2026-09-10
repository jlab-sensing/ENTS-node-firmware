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

// Enable/disable linear regression calibration
#define WATER_LEVEL_DISABLE_CALIBRATION

// Calibration constants. TODO FIX (undo) SCALE FACTOR / 0.0117
#define WATER_LEVEL_SCALING_FACTOR 0.0102
#define WATER_LEVEL_BIAS -0.662

// Water level sensors are typically marked with their maximum sensing depth.
#define WATER_LEVEL_SENSOR_MAX_DEPTH_M 2.0

// Resistor value used to read the current output from the 4-20 mA sensor.
#define WATER_LEVEL_RESISTOR_OHMS 150.0

#define WATER_LEVEL_I_MIN 0.004
#define WATER_LEVEL_I_MAX 0.020

// V_min = 4 mA * 150 Ohm = 600 mV
// V_max = 20 mA * 150 Ohm = 3000 mV
#define WATER_LEVEL_V_MIN (WATER_LEVEL_I_MIN * WATER_LEVEL_RESISTOR_OHMS)
#define WATER_LEVEL_V_MAX (WATER_LEVEL_I_MAX * WATER_LEVEL_RESISTOR_OHMS)

// Macro for converting voltages in the range [V_min, V_max] to the depth in
// range [0m, WATER_LEVEL_SENSOR_MAX_DEPTH_M]
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
