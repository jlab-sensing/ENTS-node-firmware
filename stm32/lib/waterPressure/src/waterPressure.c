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

#include "adc.h"
#include "sensor.h"
#include "sensors.h"
#include "transcoder.h"
#include "userConfig.h"

// Measured when the sensor is at atmospheric pressure (not submerged)
const double AtmosphericOffset = 2.065;

void PressureInit() {
  MX_ADC_Init();

  // B14 == p21 == channel 1
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

SEN0257Measurement PressureGetMeasurement(void) {
  SEN0257Measurement waterPressMeas;
  uint32_t value_raw = ADC_Convert_Single(ADC_CHANNEL_1);
  double value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);

  waterPressMeas.voltage = value_voltage;

  // Calibration: 250kPa range with 0.5V-4.5V output
  // Pressure (kPa) = (Vout - Voffset) * (250kPa / (4.5V - 0.5V))
  // Simplified: Pressure (kPa) = (Vout - Voffset) * 62.5
  waterPressMeas.pressure = (waterPressMeas.voltage - 0.5) * 62.5 + 33.8;
  return waterPressMeas;
}

size_t WatPress_measure(uint8_t* data, SysTime_t ts, uint32_t idx,
                        EnabledSensorMultiple* sensor) {
  // get timestamp
  SEN0257Measurement waterPressMeas = {};

  /// read voltage
  waterPressMeas = PressureGetMeasurement();
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  // voltage
  status = EncodeDoubleMeasurement(meta, waterPressMeas.voltage,
                                   SensorType_SEN0257_VOLTAGE, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  // pressure
  status =
      EncodeDoubleMeasurement(meta, waterPressMeas.pressure,
                              SensorType_SEN0257_PRESSURE, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}
