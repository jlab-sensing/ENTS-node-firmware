/**
 ******************************************************************************
 * @file     sen0308.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the capSoil.c file.
 *
 *           This library is designed to read measurements from a Capacitve Soil Moisture Sensor
 *           https://wiki.dfrobot.com/Waterproof_Capacitive_Soil_Moisture_Sensor_SKU_SEN0308
 * @date     4/8/2025
 ******************************************************************************
 */

 #ifndef LIB_CAPSOIL_INCLUDE_SEN0308_H_
 #define LIB_CAPSOIL_INCLUDE_SEN0308_H_
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include <stdio.h>
 #include <stdlib.h>
 
 #include "ads.h"

 typedef struct {
    double voltage;
    double humidity;
  } Sen0380Data;
 
 /**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   void
 ******************************************************************************
 */
 void CapSoilInit(void);
 
 /**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a CapSoil sensor.
 *
 * @param    void
 * @return   phytos_measurments
 ******************************************************************************
 */
SEN0308Measurement SEN0308GetMeasurment(void);
 
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
 size_t SEN0308_measure(uint8_t *data, SysTime_t ts);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif  // LIB_CAPSOIL_INCLUDE_SEN0308_H_