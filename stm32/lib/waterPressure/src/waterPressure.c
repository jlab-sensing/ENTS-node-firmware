/**
 ******************************************************************************
 * @file    waterPressure.c
 * @author  Stephen Taylor
 *
 * @brief   This library is designed to read measurements from a water pressure
 *          sensor from DFRobot.
 *          https://wiki.dfrobot.com/Gravity__Water_Pressure_Sensor_SKU__SEN0257
 * @date    4/23/2025
 ******************************************************************************
 */

 #include "waterPressure.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #include "transcoder.h"
 
 const float  OffSet = 0.518389;

 HAL_StatusTypeDef PressureInit() { return ADC_init(); }
 
 SEN0257Data PressureGetMeasurment() {
  SEN0257Data measurment;
   measurment.voltage = ADC_readVoltage();
   measurment.pressure = ((measurment.voltage - OffSet)) * 250 + 0.94;
   return measurment;
 }
 
 size_t WatPress_measure(uint8_t* data) {
   // get timestamp
   SysTime_t ts = SysTimeGet();
   SEN0257Data measurment = {};
 
   // read voltage
   measurment = PressureGetMeasurment();
   double adc_voltage_float = measurment.pressure;
 
   const UserConfiguration* cfg = UserConfigGet();
 
   // encode measurement
   size_t data_len = EncodeWaterPressMeasurement(
       ts.Seconds, cfg->logger_id, cfg->cell_id, adc_voltage_float, 0.0, data);
 
   // return number of bytes in serialized measurement
   return data_len;
 }