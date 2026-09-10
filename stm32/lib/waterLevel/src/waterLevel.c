/**
 ******************************************************************************
 * @file    waterLevel.c
 * @author  Eric Tran
 *
 * @brief   This library is designed to read measurements from an ALS-MPM-2F
 *          water level sensor
 *
 * @date    8/14/2026
 ******************************************************************************
 */

#include "waterLevel.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "sensor.h"
#include "transcoder.h"
#include "userConfig.h"

void WaterLevelInit(EnabledSensorMultiple* sensor) {
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

ALSMPM2FMeasurement WatLevelGetMeasurement(EnabledSensorMultiple* sensor) {
  ALSMPM2FMeasurement measurement = {0};
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
  measurement.voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);

#ifdef WATER_LEVEL_DISABLE_CALIBRATION
  measurement.meters = WATER_LEVEL_VOLTAGE_TO_METERS(measurement.voltage);
#else
  measurement.meters =
      (measurement.voltage * WATER_LEVEL_SCALING_FACTOR) + WATER_LEVEL_BIAS;
#endif

  return measurement;
}

size_t WatLevel_measure(uint8_t* data, SysTime_t ts, uint32_t idx,
                        EnabledSensorMultiple* sensor) {
  ALSMPM2FMeasurement measurement = WatLevelGetMeasurement(sensor);
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  // voltage as measured (for manual depth scaling adjustment)
  status = EncodeDoubleMeasurement(
      meta, measurement.voltage, SensorType_ALSMPM2F_VOLTAGE, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // depth in meters
  status =
      EncodeDoubleMeasurement(meta, measurement.meters,
                              SensorType_ALSMPM2F_WATER_LEVEL, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}
