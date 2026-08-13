/**
 ******************************************************************************
 * @file     waterFlowD10.h
 * @author   Eric Tran & Jack Lin
 * @brief    This file contains all the function prototypes for
 *           the waterFlowD10.c file.
 *
 *           This library is designed to read measurements from a Water Flow
 * Sensor that has 1 pulse per gallon output
 * @date     5/20/2026
 ******************************************************************************
 */

#ifndef LIB_WATERFLOW_INCLUDE_WATERFLOWD10_H_
#define LIB_WATERFLOW_INCLUDE_WATERFLOWD10_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "systime.h"
#include "transcoder.h"

/**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
void FlowD10Init(void);

/**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurement
 *           from a water Flow sensor.
 *
 * @param    void
 * @return   measurements
 ******************************************************************************
 */
D10Measurement FlowD10GetMeasurement(void);

/**
 * @brief Read water Flow sensor and serialize measurement
 *
 * The water Flow is measured via pulses. A calibration is applied
 * to convert pulses into liter per minute.
 *
 * As this is a digital sensor there is no voltage measurement.
 *
 *
 * @see SensorsPrototypeMeasure
 */
size_t WatFlowD10_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                          EnabledSensorMultiple *sensor);

#ifdef __cplusplus
}
#endif

#endif  // LIB_WATERFLOW_INCLUDE_WATERFLOWD10_H_
