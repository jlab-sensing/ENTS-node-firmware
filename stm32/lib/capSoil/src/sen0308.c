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

void CapSoilInit(EnabledSensorMultiple* sensor) {
  MX_ADC_Init();
  // map adc
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  if (sensor->index) {
    if (sensor->index == 16) {
      // channel 0
      GPIO_InitStruct.Pin = GPIO_PIN_13;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    if (sensor->index == 21) {
      // channel 1
      GPIO_InitStruct.Pin = GPIO_PIN_14;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    if (sensor->index == 22) {
      // channel 2
      GPIO_InitStruct.Pin = GPIO_PIN_3;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    if (sensor->index == 24) {
      // channel 3
      GPIO_InitStruct.Pin = GPIO_PIN_4;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    }
    if (sensor->index == 18) {
      // channel 11
      GPIO_InitStruct.Pin = GPIO_PIN_15;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
  }
}

SEN0308Measurement SEN0308GetMeasurement(EnabledSensorMultiple* sensor) {
  SEN0308Measurement capSoil;

  uint32_t channel = 0;
  if (sensor->index == 16) {
    channel = ADC_CHANNEL_0;
  } else if (sensor->index == 21) {
    channel = ADC_CHANNEL_1;
  } else if (sensor->index == 22) {
    channel = ADC_CHANNEL_2;
  } else if (sensor->index == 24) {
    channel = ADC_CHANNEL_3;
  } else if (sensor->index == 18) {
    channel = ADC_CHANNEL_11;
  }

  uint32_t value_raw = ADC_Convert_Single(channel);
  double value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);

  capSoil.voltage = value_voltage;

  // get humidity of the soil (100% when fully submerged in water)
  capSoil.humidity =
      100.0 - ((capSoil.voltage - wet_value) / (air_value - wet_value)) * 100.0;

  return capSoil;
}

size_t SEN0308_measure(uint8_t* data, SysTime_t ts, uint32_t idx,
                       EnabledSensorMultiple* sensor) {
  // get timestamp
  SEN0308Measurement capSoil;

  // read voltage
  capSoil = SEN0308GetMeasurement(sensor);
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

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
  if (sen_status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}
