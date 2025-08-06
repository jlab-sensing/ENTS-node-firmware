/**
 ******************************************************************************
 * @file     solenoid.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the solenoid.c file.
 *
 *           This library is designed to read measurements from a Water Flow Sensor
 *           https://wiki.dfrobot.com/Gravity__Water_Flow_Sensor_SKU__SEN0257
 * @date     8/6/2025
 ******************************************************************************
 */

 #ifndef LIB_SOLENOID_INCLUDE_SOLENOID_H_
 #define LIB_SOLENOID_INCLUDE_SOLENOID_H_
 
 #ifdef __cplusplus
 extern "C" {
 #endif
 
 #include <stdio.h>
 #include <stdlib.h>
 
 #include "ads.h"
 
 typedef struct {
   double flow;
 } solenoid;
 
 /**
 ******************************************************************************
 * @brief    Wrapper function for the Solenoid initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
 HAL_StatusTypeDef SolenoidInit(void);
 
 /**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a water Flow sensor.
 *
 * @param    void
 * @return   measurments
 ******************************************************************************
 */
void SolenoidOpen(void);
 
 /**
  * @brief Read water Flow sensor and serialize measurement
  *
  * The voltage output of the water Flow is measured. A calibration is applied
  * to convert voltage into a leaf wetness measurement.
  *
  * Current voltage and Flow are the same value, until a calibration
  * is obtained.
  *
  *
  * @see SensorsPrototypeMeasure
  */
 void SolenoidClose(void);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif  // LIB_CAPSOIL_INCLUDE_CAPSOIL_H_