/**
 ******************************************************************************
 * @file     watermark.h
 * @author   Jack Lin
 * @brief    This library is designed to read measurements from a Watermark
 *           200SS-VA3 adapter connected to 200SS and 200TS sensors.
 * @date     7/26/2026
 ******************************************************************************
 */

#include "watermark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "sensor.h"
#include "sensors.h"
#include "transcoder.h"
#include "userConfig.h"

void Watermark200Init(EnabledSensorMultiple *sensor) {
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

double Watermark200SS_GetMeasurement(EnabledSensorMultiple *sensor) {
  uint32_t value_raw = 0;
  double value_voltage = 0.0;
  double tension_kPa = 0.0;

  // 0 - 239 kPa from 0 - 2.8 V :: kPa = Volts / 0.0117
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
  value_raw = ADC_Convert_Single(channel);
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  tension_kPa = value_voltage / 0.0117;

  return tension_kPa;
}

double Watermark200TS_GetMeasurement(EnabledSensorMultiple *sensor) {
  uint32_t value_raw = 0;
  double value_voltage = 0.0;
  double temperature_f = 0.0;
  double WMTemp_C = 0.0;
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
  value_raw = ADC_Convert_Single(channel);
  // 20 - 132 F from 0.49 - 2.8 V :: F = 50.68 * (Volts - 0.490) + 20
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  temperature_f = 50.68 * (value_voltage - 0.490) + 20;
  WMTemp_C = (temperature_f - 32) * 5.0 / 9.0;

  return WMTemp_C;
}
size_t Watermark200SS_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                              EnabledSensorMultiple *sensor) {
  double WM_kPA = 0.0;
  WM_kPA = Watermark200SS_GetMeasurement(sensor);
  const UserConfiguration *cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  status = EncodeDoubleMeasurement(
      meta, WM_kPA, SensorType_WATERMARK200SS_SOIL_TENSION, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // return number of bytes in serialized measurement
  return data_len;
}

size_t Watermark200TS_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                              EnabledSensorMultiple *sensor) {
  double temp_c = 0.0;
  temp_c = Watermark200TS_GetMeasurement(sensor);
  const UserConfiguration *cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  // Temperature
  status = EncodeDoubleMeasurement(meta, temp_c,
                                   SensorType_WATERMARK200TS_SOIL_TEMPERATURE,
                                   data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}