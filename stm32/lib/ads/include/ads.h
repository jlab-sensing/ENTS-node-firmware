/**
 ******************************************************************************
 * @file     ads.h
 * @author   Stephen Taylor
 * @brief    This file contains all the function prototypes for
 *           the ads.c file
 * @date     11/27/2023
 ******************************************************************************
 * Copyright [2023] <Stephen Taylor>
 */

#ifndef LIB_ADS_INCLUDE_ADS_H_
#define LIB_ADS_INCLUDE_ADS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "i2c.h"
#include "stm32_systime.h"
#include "timestamp.h"
#include "transcoder.h"
#include "usart.h"

/**
 * @ingroup stm32
 * @defgroup ads ADS1219
 * @brief Library for interfacing with the ADS1219 ADC
 *
 * This library is designed to read measurements from the ADS1219 ADC. When not
 * actively taking measurements, the analog frontend is power down to reduce
 * parasitic current.
 *
 * Library expected I2C and GPIO to be initialized before use.
 *
 * Example: @ref example_adc.c
 *
 * Datasheet: https://www.ti.com/product/ADS1219
 *
 *
 * @{
 */

/**
******************************************************************************
* @brief    This function starts up the ADS1219
*
*           This function is a wrapper for the STM32 HAl I2C library. The
*ADS1219 uses I2C the I2C communication protocol. This function configures then
*ADS1219 for single read mode. Note: the ADS1219 requires a minimum of 500us
*when it is powered on.
*
* @param    void
* @return   HAL_StatusTypeDef
******************************************************************************
*/
HAL_StatusTypeDef ADC_init(void);

/**
******************************************************************************
* @brief    This function reads the current ADC voltage value.
*
*           This function is a wrapper for the STM32 HAl I2C library. The
*ADS1219 uses I2C the I2C communication protocol. This version simply chops the
*noisy bits.
*
* @param    void
* @return   double, current ADC reading in microvolts
******************************************************************************
*/
double ADC_readVoltage(void);

/**
******************************************************************************
* @brief    This function reads the current ADC ampere value.
*
*           This function is a wrapper for the STM32 HAl I2C library. The
*ADS1219 uses I2C the I2C communication protocol. This version simply chops the
*noisy bits.
*
* @param    void
* @return   double, current ADC reading in microamps
******************************************************************************
*/
double ADC_readCurrent(void);

/**
******************************************************************************
* @brief    This function probes the ADS12 to see if it is responsive.
*
* @param    void
* @return   HAL_StatusTypeDef
*******************************************f***********************************
*/
HAL_StatusTypeDef ADC_probe(void);

/**
******************************************************************************
* @brief    This function encodes the ADS1219 power measurments into protobuf
*
* @param    *data
* @return   size_t
*******************************************f***********************************
*/
size_t ADC_measure(uint8_t *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_ADS_INCLUDE_ADS_H_
