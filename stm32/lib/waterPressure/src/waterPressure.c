/**
 ******************************************************************************
 * @file    waterPressure.c
 * @author  Caden Jacobs
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
 
 const double  OffSet = 0.5104;

 HAL_StatusTypeDef PressureInit() { return ADC_init(); }
 
  SEN0257Meas PressureGetMeasurment() {
    SEN0257Meas measurment;
    measurment.voltage = ADC_readVoltage();
    measurment.pressure = (measurment.voltage - OffSet) * 250;
    return measurment;
 }
 
 size_t WatPress_measure(uint8_t* data) {
   // get timestamp
   SysTime_t ts = SysTimeGet();
   SEN0257Meas measurment = {};
 
   /// read voltage
   measurment = PressureGetMeasurment();
   double float_pressure = measurment.pressure;
   double float_voltage = measurment.voltage;
   const UserConfiguration* cfg = UserConfigGet();
 
   // encode measurement
   size_t data_len = EncodeWaterPressMeasurement(
       ts.Seconds, cfg->logger_id, cfg->cell_id, float_voltage, float_pressure, data);
 
   // return number of bytes in serialized measurement
   return data_len;
 }