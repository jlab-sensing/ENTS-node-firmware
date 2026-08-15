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

#include "sensor.h"
#include "sensors.h"
#include "transcoder.h"
#include "userConfig.h"
#include "adc.h"

// Measured when the sensor is at atmospheric pressure (not submerged)
const double AtmosphericOffset = 2.065;

HAL_StatusTypeDef WaterLevelInit() { 
  // return ADC_init(); }

   MX_ADC_Init();
  // map adc
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

ALSMPM2FMeasurement WatLevelGetMeasurement() {
  ALSMPM2FMeasurement waterLevelMeas;
  waterLevelMeas.voltage = ADC_readVoltage();


  uint32_t value_raw = 0;
  double value_voltage = 0.0;
  double waterLevel_meter = 0.0;

  uint32_t channel = ADC_CHANNEL_3;
  value_raw = ADC_Convert_Single(channel);
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
  waterLevelMeas.voltage = value_voltage / 0.0117; // TODO remove this line later

  // Calibration: 250kPa range with 0.5V-4.5V output
  // Pressure (kPa) = (Vout - Voffset) * (250kPa / (4.5V - 0.5V))
  // Simplified: Pressure (kPa) = (Vout - Voffset) * 62.5

// TO DO: CALLIBRATE BASIC WATER LEVEL SENSOR USIGN LLSR
  //   waterLevelMeas.voltage = (waterLevelMeas.voltage - 0.5) * 62.5 + 33.8;


  return waterLevelMeas;
}

size_t WatLevel_measure(uint8_t* data, SysTime_t ts, uint32_t idx) {
  // get timestamp
  ALSMPM2FMeasurement waterLevelMeas = {};

  /// read voltage
  waterLevelMeas = WatLevelGetMeasurement();
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  // voltage
  status = EncodeDoubleMeasurement(meta, waterLevelMeas.voltage,
                                   SensorType_SEN0257_VOLTAGE, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

//   // pressure
//   status =
//       EncodeDoubleMeasurement(meta, waterPressMeas.pressure,
//                               SensorType_SEN0257_PRESSURE, data, &data_len);
//   if (status != SENSOR_OK) {
//     return -1;
//   }
  
  // return number of bytes in serialized measurement
  return data_len;
}
