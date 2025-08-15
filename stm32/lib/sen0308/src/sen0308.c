/**
******************************************************************************
* @file    capSoil.c
* @author  Caden Jacobs
*
* @brief   This library is designed to read measurements from a PHYTOS-31
*          sensor from METER.
*          https://metergroup.com/products/phytos-31/
* @date    4/18/2024
******************************************************************************
*/

#include "sen0308.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
#include "transcoder.h"
#include "adc.h"

//calibration values
const double air_value = 2.372;
const double wet_value = 0.036;
const double intervals = (air_value - wet_value)/3;
 
HAL_StatusTypeDef CapSoilInit() { return ADC_init(); }

 
SEN0308_measurments SEN0308GetMeasurment() {
  SEN0308_measurments measurments;
   
   measurments.capSoil_raw = ADC_readVoltage();

   //get humidity of the soil (100% when fully submerged)
   measurments.capSoil_calibrated = 100.0 - ((measurments.capSoil_raw - wet_value) / (air_value - wet_value)) * 100.0;
   
   return measurments;
 }
 
 size_t SEN0308_measure(uint8_t* data) {
   // get timestamp
   SysTime_t ts = SysTimeGet();
   SEN0308_measurments measurment;
 
   // read voltage
   measurment = SEN0308GetMeasurment();
   double humditiy_float = measurment.capSoil_calibrated;
   double voltage_float = measurment.capSoil_raw;
 
   const UserConfiguration* cfg = UserConfigGet();
 
   // encode measurement
   size_t data_len = EncodeSEN0308Measurement( //check measure!!
       ts.Seconds, cfg->logger_id, cfg->cell_id, voltage_float, humditiy_float, data);
 
   // return number of bytes in serialized measurement
   return data_len;
 }