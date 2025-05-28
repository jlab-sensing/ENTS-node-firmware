// Copyright 2024 Jlab sensing

/**
 * @file     phytos31.h
 * @author   Stephen Taylor
 * @brief    This file contains all the function prototypes for
 *           the phytos31.c file.
 *
 * @date     4/18/2024
 */

#ifndef LIB_PHYTOS31_INCLUDE_PHYTOS31_H_
#define LIB_PHYTOS31_INCLUDE_PHYTOS31_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "ads.h"

/**
 * @ingroup stm32
 * @defgroup phytos31 Phytos31
 * @brief Library for interfacing with the PHYTOS-31 sensor
 *
 * This library is designed to read measurements from a PHYTOS-31 sensor from
 * METER.
 *
 * https://metergroup.com/products/phytos-31/

 * Examples:
 * - @ref example_phytos.c
 *
 * @{
 */

typedef struct {
  double phytos31_raw;
  double phytos31_calibrated;
} phytos_measurments;

/**
******************************************************************************
* @brief    Wrapper function for the ADC initilization.
*
* @param    void
* @return   HAL_StatusTypeDef
******************************************************************************
*/
HAL_StatusTypeDef Phytos31Init(void);

/**
******************************************************************************
* @brief    Returns both the raw voltage value and a calibrated measurment
*           from a PHYTOS31 sensor.
*
* @param    void
* @return   phytos_measurments
******************************************************************************
*/
phytos_measurments Phytos31GetMeasurment(void);

/**
 * @brief Read PHYTOS31 sensor and serialize measurement
 *
 * The voltage output of the PHYTOS31 is measured. A calibration is applied
 * to convert voltage into a leaf wetness measurement.
 *
 * Current voltage and leaf wetness are the same value, until a calibration
 * is obtained.
 *
 * @note Implemented for the sensors library
 *
 * @see SensorsPrototypeMeasure
 */
size_t Phytos31_measure(uint8_t *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_PHYTOS31_INCLUDE_PHYTOS31_H_
