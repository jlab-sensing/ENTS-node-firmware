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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transcoder.h"

HAL_StatusTypeDef Phytos31Init() { return ADC_init(); }

phytos_measurments Phytos31GetMeasurment() {
  phytos_measurments measurments;
  measurments.phytos31_raw = ADC_readVoltage();
  return measurments;
}

size_t Phytos31_measure(uint8_t *data) {
  // get timestamp
  SysTime_t ts = SysTimeGet();
  phytos_measurments measurment;

  // read voltage
  measurment = Phytos31GetMeasurment();
  double adc_voltage_float = measurment.phytos31_raw / 1000.;

  const UserConfiguration *cfg = UserConfigGet();

  // encode measurement
  size_t data_len = EncodePhytos31Measurement(
      ts.Seconds, cfg->logger_id, cfg->cell_id, adc_voltage_float, 0.0, data);

  // return number of bytes in serialized measurement
  return data_len;
}
