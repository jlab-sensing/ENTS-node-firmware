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

static double WM1_kPa = 0.0;
static double WM2_kPa = 0.0;
static double WM3_kPa = 0.0;
static double WMTemp_C = 0.0;

void Watermark200SSVA3_Init(void) {
  MX_ADC_Init();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void Watermark200SSVA3_GetMeasurement(void) {
  uint32_t value_raw = 0;
  double value_voltage = 0.0;
  double tension_kPa = 0.0;
  double temperature_f = 0.0;

  // 0 - 239 kPa from 0 - 2.8 V :: kPa = Volts / 0.0117
  value_raw = ADC_Convert_Single(ADC_CHANNEL_1);
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  WM1_kPa = value_voltage / 0.0117;

  value_raw = ADC_Convert_Single(ADC_CHANNEL_2);
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  WM2_kPa = value_voltage / 0.0117;

  value_raw = ADC_Convert_Single(ADC_CHANNEL_3);
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  WM3_kPa = value_voltage / 0.0117;

  // 20 - 132 F from 0.49 - 2.8 V :: F = 50.68 * (Volts - 0.490) + 20
  value_raw = ADC_Convert_Single(ADC_CHANNEL_11);
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  temperature_f = 50.68 * (value_voltage - 0.490) + 20;
  WMTemp_C = (temperature_f - 32) * 5.0 / 9.0;
}

double Watermark200SSVA3_GetWM1(void) { return WM1_kPa; }
double Watermark200SSVA3_GetWM2(void) { return WM2_kPa; }
double Watermark200SSVA3_GetWM3(void) { return WM3_kPa; }
double Watermark200SSVA3_GetWMTemp(void) { return WMTemp_C; }

size_t Watermark200SSVA3_measure(uint8_t* data, SysTime_t ts, uint32_t idx) {
  Watermark200SSVA3_GetMeasurement();
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  // WM1
  meta.cell_id = cfg->enabled_sensors_multiple[0].cell_id;
  status = EncodeDoubleMeasurement(meta, Watermark200SSVA3_GetWM1(),
                                   SensorType_WATERMARK200SS_SOIL_TENSION, data,
                                   &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);
  // WM2
  meta.cell_id = cfg->enabled_sensors_multiple[1].cell_id;
  status = EncodeDoubleMeasurement(meta, Watermark200SSVA3_GetWM2(),
                                   SensorType_WATERMARK200SS_SOIL_TENSION, data,
                                   &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);
  // WM3
  meta.cell_id = cfg->enabled_sensors_multiple[2].cell_id;
  status = EncodeDoubleMeasurement(meta, Watermark200SSVA3_GetWM3(),
                                   SensorType_WATERMARK200SS_SOIL_TENSION, data,
                                   &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // Temperature
  meta.cell_id = cfg->enabled_sensors_multiple[3].cell_id;
  status = EncodeDoubleMeasurement(meta, Watermark200SSVA3_GetWMTemp(),
                                   SensorType_WATERMARK200TS_SOIL_TEMPERATURE,
                                   data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}
