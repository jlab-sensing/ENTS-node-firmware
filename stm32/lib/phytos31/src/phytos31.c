// Copyright 2024 Jlab sensing

/**
 ******************************************************************************
 * @file    sdi12.c
 * @author  Stephen Taylor
 *
 * @brief   This library is designed to read measurements from a PHYTOS-31
 *          sensor from METER.
 *          https://metergroup.com/products/phytos-31/
 * @date    4/18/2024
 ******************************************************************************
 */

#include "phytos31.h"

// standard includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// user includes
#include "transcoder.h"
#include "userConfig.h"

HAL_StatusTypeDef Phytos31Init(EnabledSensorMultiple *sensor) {
  const UserConfiguration *cfg = UserConfigGet();
  return ADC_init(sensor, cfg->Voltage_Slope, cfg->Voltage_Offset,
                  cfg->Current_Slope, cfg->Current_Offset);
}

phytos_measurements Phytos31GetMeasurement(EnabledSensorMultiple *sensor) {
  phytos_measurements measurements;
  measurements.phytos31_raw = ADC_readVoltage(sensor);
  return measurements;
}

size_t Phytos31_measure(uint8_t *data, SysTime_t ts, uint32_t idx,
                        EnabledSensorMultiple *sensor) {
  phytos_measurements measurement;

  // read voltage
  measurement = Phytos31GetMeasurement(sensor);
  double adc_voltage_float = measurement.phytos31_raw / 1000.;

  const UserConfiguration *cfg = UserConfigGet();

  // encode measurement
  size_t data_len =
      EncodePhytos31Measurement(ts.Seconds, cfg->logger_id, sensor->cell_id,
                                adc_voltage_float, 0.0, data);

  // return number of bytes in serialized measurement
  return data_len;
}