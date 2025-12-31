/**
 ******************************************************************************
 * @file     waterPressure.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the waterPressure.c file.
 *
 *           This library is designed to read measurements from a Water Pressure
 *Sensor https://wiki.dfrobot.com/Gravity__Water_Pressure_Sensor_SKU__SEN0257
 * @date     4/23/2025
 ******************************************************************************
 */

#ifndef LIB_WATERPRESSURE_INCLUDE_WATERPRESSURE_H_
#define LIB_WATERPRESSURE_INCLUDE_WATERPRESSURE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ads.h"

/**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
HAL_StatusTypeDef PressureInit(void);

/**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a water pressure sensor.
 *
 * @param    void
 * @return   measurments
 ******************************************************************************
 */
SEN0257Measurement PressureGetMeasurment(void);

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

#endif  // LIB_WATERPRESSURE_INCLUDE_WATERPRESSURE_H_
