/**
 ******************************************************************************
 * @file     capSoil.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the capSoiil.c file.
 *
 *           This library is designed to read measurements from a Capacitve Soil Moisture Sensor
 *           [link]
 * @date     4/8/2025
 ******************************************************************************
 */

 #ifndef LIB_CAPSOIL_INCLUDE_CAPSOIL_H_
 #define LIB_CAPSOIL_INCLUDE_CAPSOIL_H_
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include <stdio.h>
 #include <stdlib.h>
 
 #include "ads.h"
 
 typedef struct {
   double capSoil_raw;
   double capSoil_calibrated;
 } SEN0308_measurments;
 
 /**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
 HAL_StatusTypeDef CapSoilInit(void);
 
 /**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a CapSoil sensor.
 *
 * @param    void
 * @return   phytos_measurments
 ******************************************************************************
 */
SEN0308_measurments SEN0308GetMeasurment(void);
 
 /**
  * @brief Read CapSoil sensor and serialize measurement
  *
  * The voltage output of the CapSoil is measured. A calibration is applied
  * to convert voltage into a leaf wetness measurement.
  *
  * Current voltage and leaf wetness are the same value, until a calibration
  * is obtained.
  *
  * @note Implemented for the sensors library
  *
  * @see SensorsPrototypeMeasure
  */
 size_t SEN0308_measure(uint8_t *data);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif  // LIB_CAPSOIL_INCLUDE_CAPSOIL_H_