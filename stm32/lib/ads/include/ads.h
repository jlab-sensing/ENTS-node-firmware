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

#define ADS12_WRITE 0x80
#define ADS12_READ 0x81

#define ADS12_START_CODE 0x08
#define ADS12_RESET_CODE 0x06
#define ADS12_READ_DATA_CODE 0x10

extern int32_t previous_voltage_reading;
extern int32_t previous_current_reading;


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
* @brief    This function reconfigures the ADS1219 based on the parameter
*reg_data
*
*
* @param    reg_data
* @return   HAL_StatusTypeDef
******************************************************************************
*/
HAL_StatusTypeDef ADC_configure(uint8_t reg_data);

/**
******************************************************************************
* @brief    Helper function to convert the int32_t 2's complement reading to 
* 1's complement
*
*
* @param    reading
* @return   uint32_t, the converted 1's complement reading
******************************************************************************
*/
int32_t ADC_twos_to_ones(int32_t delta);

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
int32_t ADC_readVoltage(void);

/**
******************************************************************************
* @brief    This function reads the current ADC ampere value.
*
*           This function is a wrapper for the STM32 HAl I2C library. The
*ADS1219 uses I2C the I2C communication protocol. This version simply chops the
*noisy bits.
*
* @param    void
* @return   int32_t, current ADC reading in microamps
******************************************************************************
*/
int32_t ADC_readCurrent(void);

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

#ifdef __cplusplus
}
#endif

#endif  // LIB_ADS_INCLUDE_ADS_H_
