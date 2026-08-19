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

#include "ads.h"

#define WATER_LEVEL_SCALING_FACTOR 0.0102
#define WATER_LEVEL_BIAS -0.662

/**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
void WaterLevelInit(void);

/**
 ******************************************************************************
 * @brief    Returns both thecalibrated measurement
 *           from a water level sensor.
 *
 * @param    void
 * @return   measurements
 ******************************************************************************
 */
ALSMPM2FMeasurement WatLevelGetMeasurement(void);

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
