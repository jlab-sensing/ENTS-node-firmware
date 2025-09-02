/**
******************************************************************************
* @file    sen0308.c
* @author  Caden Jacobs
*
* @brief   This library is designed to read measurements from a Capacitve
*          Soil Moisture Sensor
*          https://wiki.dfrobot.com/Waterproof_Capacitive_Soil_Moisture_Sensor_SKU_SEN0308
* @date    4/18/2024
******************************************************************************
*/

#include "sen0308.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "transcoder.h"

// Calibration Values
// The air_value was determined by reading ADC within air in jLab
// The wet_value was determind by reading ADC when submerged in a cup
const double air_value = 2.372;
const double wet_value = 0.036;
const double intervals = (air_value - wet_value) / 3;

void CapSoilInit() { return ADC_init(); }

SEN03808Measurement SEN0308GetMeasurment() {
  SEN03808Measurement capSoil;

  capSoil.voltage = ADC_readVoltage();

  // get humidity of the soil (100% when fully submerged in water)
  capSoil.humidity =
      100.0 - ((capSoil.voltage - wet_value) / (air_value - wet_value)) * 100.0;

  return capSoil;
}

size_t SEN0308_measure(uint8_t* data, SysTime_t ts) {
  // get timestamp
  SEN03808Measurement capSoil;

  // read voltage
  capSoil = SEN0308GetMeasurment();
  const UserConfiguration* cfg = UserConfigGet();

  // encode measurement
  size_t data_len =
      EncodeSEN0308Measurement(ts.Seconds, cfg->logger_id, cfg->cell_id,
                               capSoil.voltage, capSoil.humidity, data);

  // return number of bytes in serialized measurement
  return data_len;
}