/**
 ******************************************************************************
 * @file     watermark.h
 * @author   Jack Lin
 * @brief    This library is designed to read measurements from a Watermark
 *           200SS-VA3 adapter connected to 200SS and 200TS sensors.
 * @date     7/26/2026
 ******************************************************************************
 */

#ifndef LIB_WATERMARK_INCLUDE_WATERMARK_H_
#define LIB_WATERMARK_INCLUDE_WATERMARK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "sensors.h"
#include "stm32_systime.h"

// Reading temperature can be done using the Watermark adapter or a resistor
// divider. The Watermark 200TS is a 10 kOhm NTC thermistor.
// #define WATERMARK_200TS_VA3_ADAPTER
#define WATERMARK_200TS_RESISTOR_DIVIDER

#if defined(WATERMARK_200TS_VA3_ADAPTER) && \
    defined(WATERMARK_200TS_RESISTOR_DIVIDER)
#error Enable only one of the Watermark 200TS measurement modes in watermark.h.
#endif

#ifdef WATERMARK_200TS_RESISTOR_DIVIDER
#define WATERMARK_200TS_RESISTOR_DIVIDER_MIN_TEMPERATURE_C -55
#define WATERMARK_200TS_RESISTOR_DIVIDER_MAX_TEMPERATURE_C 150

// Choose a fixed resistor centering the range of expected readings.
// 3300 52C
// 4700 43C
// 10000 25C
#define WATERMARK_200TS_RESISTOR_DIVIDER_FIXED_R 3300
#endif

/**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *           Calls MX_ADC_Init().
 *           Assumes WM1 channel 1 (21), WM2 channel 2 (22), WM3 channel 3 (24),
 *           temperature channel 11 (18).
 *
 * @todo     Initialize based on selected ADC channels from user config (index).
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
void Watermark200Init(EnabledSensorMultiple *sensor);

/**
 ******************************************************************************
 * @brief    Updates global variables for soil tension and soil temperature.
 *
 * @todo     Struct to return the data directly to the caller.
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
double Watermark200TS_GetMeasurement(EnabledSensorMultiple *sensor);
double Watermark200SS_GetMeasurement(EnabledSensorMultiple *sensor);

/**
 * @brief Read all three soil tensiometers and the soil temperature sensor and
 *        serialize measurement.
 *
 * @see SensorsPrototypeMeasure
 */
size_t Watermark200SS_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                              EnabledSensorMultiple *sensor);
size_t Watermark200TS_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                              EnabledSensorMultiple *sensor);

#ifdef __cplusplus
}
#endif

#endif  // LIB_WATERMARK_INCLUDE_WATERMARK_H_
