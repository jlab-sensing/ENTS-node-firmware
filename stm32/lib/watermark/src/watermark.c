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

#ifdef WATERMARK_200TS_RESISTOR_DIVIDER
#define WATERMARK_200TS_RESISTOR_DIVIDER_T_OFFSET_C \
  WATERMARK_200TS_RESISTOR_DIVIDER_MIN_TEMPERATURE_C
// r_lookup[temperature in C offset by +55] = thermistor_resistance
// -55C to +150C inclusive
// ex. r_lookup[80] = 10000 (R at 25C)
const double r_lookup[] = {
    963849.33, 895318.87, 832108.29, 773771.65, 719903.28, 670133.66, 624125.96,
    581573,    542194.46, 505734.38, 471958.91, 440654.27, 411624.95, 384692,
    359691.55, 336473.5,  314900.23, 294845.52, 276193.56, 258838,    242681.16,
    227633.25, 213611.71, 200540.58, 188349.94, 176975.41, 166357.65, 156441.97,
    147177.92, 138518.96, 130422.13, 122847.74, 115759.11, 109122.35, 102906.11,
    97081.38,  91621.3,   86501,     81697.45,  77189.28,  72956.7,   68981.32,
    65246.1,   61735.2,   58433.93,  55328.6,   52406.53,  49655.89,  47065.7,
    44625.72,  42326.42,  40158.95,  38115.02,  36186.93,  34366.5,   32650,
    31029.25,  29498.39,  28051.95,  26684.81,  25392.19,  24169.64,  23012.98,
    21918.31,  20881.97,  19900.54,  18970.82,  18089.82,  17254.73,  16462.9,
    15711.88,  14999.34,  14323.12,  13681.17,  13071.57,  12492.53,  11942.36,
    11419.48,  10922.37,  10449.66,  10000,     9572.16,   9164.98,   8777.34,
    8408.21,   8056.61,   7721.63,   7402.39,   7098.07,   6807.91,   6531.17,
    6267.17,   6015.25,   5774.8,    5545.25,   5326.04,   5116.67,   4916.63,
    4725.47,   4542.75,   4368.06,   4201,      4041.21,   3888.33,   3742.03,
    3602,      3467.93,   3339.55,   3216.59,   3098.79,   2985.91,   2877.72,
    2774,      2674.55,   2579.18,   2487.69,   2399.91,   2315.68,   2234.83,
    2157.21,   2082.67,   2011.09,   1942.32,   1876.25,   1812.76,   1751.73,
    1693.06,   1636.65,   1582.4,    1530.21,   1480,      1431.69,   1384.99,
    1340.17,   1297.04,   1255.51,   1215.54,   1177.04,   1139.97,   1104.25,
    1069.84,   1036.68,   1004.71,   973.89,    944.18,    915.52,    887.88,
    861.21,    835.48,    810.64,    786.67,    763.52,    741.18,    719.59,
    698.74,    678.6,     659.14,    640.33,    622.15,    604.57,    587.58,
    571.14,    555.25,    539.87,    524.99,    510.6,     496.66,    483.18,
    470.12,    457.48,    445.24,    433.38,    421.9,     410.77,    399.99,
    389.54,    379.41,    369.6,     360.08,    350.85,    341.9,     333.22,
    324.8,     316.63,    308.7,     301.01,    293.55,    286.3,     279.26,
    272.43,    265.8,     259.36,    253.1,     247.02,    241.12,    235.38,
    229.8,     224.38,    219.12,    214,       209.02,    204.17,    199.47,
    194.89,    190.43,    186.1};
double v_lookup[sizeof(r_lookup) / sizeof(double)];
#endif

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
#ifdef WATERMARK_200TS_RESISTOR_DIVIDER
  // Calculate voltage lookup table given the fixed resistor value
  const double R = WATERMARK_200TS_RESISTOR_DIVIDER_FIXED_R;
  for (int i = 0; i < (sizeof(v_lookup) / sizeof(double)); i++) {
    v_lookup[i] = 3.3 * R / (R + v_lookup[i]);
  }
#endif
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
  value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);

#ifdef WATERMARK_200TS_VA3_ADAPTER
  // 20 - 132 F from 0.49 - 2.8 V :: F = 50.68 * (Volts - 0.490) + 20
  temperature_f = 50.68 * (value_voltage - 0.490) + 20;
  WMTemp_C = (temperature_f - 32) * 5.0 / 9.0;
#elif defined WATERMARK_200TS_RESISTOR_DIVIDER
  // Search through the voltage table until we find an entry matching
  // the voltage we measured. Then, assume linear relationship between
  // neighboring entries.
  for (int i = 0; i < (sizeof(v_lookup) / sizeof(double)); i++) {
    // Note: NTC thermistor, search forward and stop when entry exceeds.
    if (value_voltage < v_lookup[i]) {
      if (i == 0) {
        // Below minimum temperature (voltage) range, outside of table.
        WMTemp_C = WATERMARK_200TS_RESISTOR_DIVIDER_MIN_TEMPERATURE_C;
        break;
      }

      // Add the offset to the previous index to obtain the temperature that
      // was overshot. Find where the measurement is in between the two entries
      // (units V/V), and add that to the temperature assuming a linear
      // relationship between the two neighboring entries (1C * V/V).

      WMTemp_C =
          (i - 1 + WATERMARK_200TS_RESISTOR_DIVIDER_T_OFFSET_C) +
          ((value_voltage - v_lookup[i - 1]) / (v_lookup[i] - v_lookup[i - 1]));
      break;
    }
    if (i == (sizeof(v_lookup) / sizeof(double))) {
      // Above maximum temperature (voltage) range, outside of table.
      WMTemp_C = WATERMARK_200TS_RESISTOR_DIVIDER_MAX_TEMPERATURE_C;
      break;
    }
  }
#endif

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