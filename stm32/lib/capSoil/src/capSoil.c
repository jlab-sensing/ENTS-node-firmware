/**
 ******************************************************************************
 * @file    capSoil.c
 * @author  Stephen Taylor
 *
 * @brief   This library is designed to read measurements from a PHYTOS-31
 *          sensor from METER.
 *          https://metergroup.com/products/phytos-31/
 * @date    4/18/2024
 ******************************************************************************
 */

 #include "capSoil.h"

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 
 #include "transcoder.h"
 
 HAL_StatusTypeDef CapSoilInit() { return ADC_init(); }
 
 capSoil_measurments CapSoilGetMeasurment() {
   capSoil_measurments measurments;
   measurments.capSoil_raw = ADC_readVoltage();
   measurments.capSoil_calibrated = ((measurments.capSoil_raw)*3.3);
   return measurments;
 }
 
 size_t CapSoil_measure(uint8_t* data) {
   // get timestamp
   SysTime_t ts = SysTimeGet();
   capSoil_measurments measurment;
 
   // read voltage
   measurment = CapSoilGetMeasurment();
   double adc_voltage_float = measurment.capSoil_calibrated;
 
   const UserConfiguration* cfg = UserConfigGet();
 
   // encode measurement
   size_t data_len = EncodeCapSoilMeasurement( //check measure!!
       ts.Seconds, cfg->logger_id, cfg->cell_id, adc_voltage_float, 0.0, data);
 
   // return number of bytes in serialized measurement
   return data_len;
 }