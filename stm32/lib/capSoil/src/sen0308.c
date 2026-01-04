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
#include "sensor.h"
#include "sensors.h"
#include "transcoder.h"
#include "userConfig.h"

// Calibration Values
// The air_value was determined by reading ADC within air in jLab
// The wet_value was determind by reading ADC when submerged in a cup
const double air_value = 2.372;
const double wet_value = 0.036;
const double intervals = (air_value - wet_value) / 3;

void CapSoilInit() { ADC_init(); }

SEN0308Measurement SEN0308GetMeasurment() {
  SEN0308Measurement capSoil;

  capSoil.voltage = ADC_readVoltage();

  // get humidity of the soil (100% when fully submerged in water)
  capSoil.humidity =
      100.0 - ((capSoil.voltage - wet_value) / (air_value - wet_value)) * 100.0;

  return capSoil;
}

size_t SEN0308_measure(uint8_t* data, SysTime_t ts, uint32_t idx) {
  // get timestamp
  SEN0308Measurement capSoil;

  // read voltage
  capSoil = SEN0308GetMeasurment();
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  // variables for the next block
  size_t data_len = 0;
  SensorStatus sen_status = SENSOR_OK;

  sen_status = EncodeDoubleMeasurement(
      meta, capSoil.voltage, SensorType_SEN0308_VOLTAGE, data, &data_len);
  if (sen_status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  sen_status = EncodeDoubleMeasurement(
      meta, capSoil.humidity, SensorType_SEN0308_HUMIDITY, data, &data_len);

  // return number of bytes in serialized measurement
  return data_len;
}
