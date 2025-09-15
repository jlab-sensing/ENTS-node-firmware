/**
 ******************************************************************************
 * @file     waterFlow.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the waterFlow.c file.
 *
 *           This library is designed to read measurements from a Water Flow
 *Sensor
 *           https://www.danomsk.ru/upload/iblock/43d/193917_3b664efb7b37f7ae8ea1eea40978a265.pdf
 * @date     7/31/2025
 ******************************************************************************
 */

#ifndef LIB_WATERFLOW_INCLUDE_WATERFLOW_H_
#define LIB_WATERFLOW_INCLUDE_WATERFLOW_H_

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
void FlowInit(void);

/**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a water Flow sensor.
 *
 * @param    void
 * @return   measurments
 ******************************************************************************
 */
YFS210CMeasurement FlowGetMeasurment(void);

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
size_t WatFlow_measure(uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif  // LIB_WATERFLOW_INCLUDE_WATERFLOW_H_