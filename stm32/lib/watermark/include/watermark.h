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
#include "stm32_systime.h"

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
void Watermark200SSVA3_Init(void);

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
void Watermark200SSVA3_GetMeasurement(void);

double Watermark200SSVA3_GetWM1(void);
double Watermark200SSVA3_GetWM2(void);
double Watermark200SSVA3_GetWM3(void);
double Watermark200SSVA3_GetWMTemp(void);

/**
 * @brief Read all three soil tensiometers and the soil temperature sensor and
 *        serialize measurement.
 *
 * @see SensorsPrototypeMeasure
 */
size_t Watermark200SSVA3_measure(uint8_t *data, SysTime_t ts, uint32_t idx);

#ifdef __cplusplus
}
#endif

#endif  // LIB_WATERMARK_INCLUDE_WATERMARK_H_
