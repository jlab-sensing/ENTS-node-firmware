/**
 ******************************************************************************
 * @file     waterFlow.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the waterFlow.c file.
 *
 *           This library is designed to read measurements from a Water Flow Sensor
 *           https://wiki.dfrobot.com/Gravity__Water_Flow_Sensor_SKU__SEN0257
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
 
 typedef struct {
   double flow;
 } waterFlow;
 
 /**
 ******************************************************************************
 * @brief    Wrapper function for the ADC initilization.
 *
 * @param    void
 * @return   HAL_StatusTypeDef
 ******************************************************************************
 */
 HAL_StatusTypeDef FlowInit(void);
 
 /**
 ******************************************************************************
 * @brief    Returns both the raw voltage value and a calibrated measurment
 *           from a water Flow sensor.
 *
 * @param    void
 * @return   measurments
 ******************************************************************************
 */
waterFlow FlowGetMeasurment(void);
 
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
 size_t WatFlow_measure(uint8_t *data);
 
 #ifdef __cplusplus
 }
 #endif
 
 #endif  // LIB_CAPSOIL_INCLUDE_CAPSOIL_H_