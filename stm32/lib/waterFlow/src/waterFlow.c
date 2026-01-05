/**
 ******************************************************************************
 * @file    waterFlow.c
 * @author  Caden Jacobs
 *
 * @brief   This library is designed to read measurements from a Water Flow
 *Sensor
 *          https://www.danomsk.ru/upload/iblock/43d/193917_3b664efb7b37f7ae8ea1eea40978a265.pdf
 *
 * @date    7/31/2025
 ******************************************************************************
 */

#include "waterFlow.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sensor.h"
#include "sensors.h"
#include "stm32wlxx_hal_def.h"
#include "transcoder.h"
#include "usart.h"
#include "userConfig.h"

#define FLOW_AVG_COUNT 5

// Variables
static volatile float last_flow_lpm = 0;
static volatile unsigned long pulse_count = 0;
SysTime_t currentTime;
SysTime_t lastTime;
static volatile float flow_history[FLOW_AVG_COUNT] = {0};
uint8_t flow_index = 0;

// For every one liter of water that passes through the sensor in one minute,
// there are 450 pulses. Therefore the calibration factor becomes [450/60 = 7.5]
const float calibration_factor = 7.5;

void FlowInit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Configure PA10 as rising edge EXTI input (water flow sensor output)
  __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable  clock for port A

  // reset pins
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

  // Configure PIN 10 on Port A (GPIO input)
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  // interrupt on rising edge
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  // Get INIT Times
  currentTime = SysTimeGet();
  lastTime = currentTime;
}

YFS210CMeasurement FlowGetMeasurement() {
  // get time
  currentTime = SysTimeGet();
  SysTime_t diff = SysTimeSub(currentTime, lastTime);
  // Always calculate flow, not just every 100ms
  uint32_t pulses = pulse_count;

  YFS210CMeasurement flowMeas;

  // Calculate liters per minute based on actual time elapsed
  float time_elapsed_minutes =
      (float)diff.SubSeconds / 6000.0f;  // Convert subseconds to minutes
  if (time_elapsed_minutes > 0) {
    last_flow_lpm = ((float)pulses / calibration_factor) / time_elapsed_minutes;
    pulse_count = 0;  // Reset after calculation
    lastTime = currentTime;
  }

  // Update history and calculate average
  flow_history[flow_index] = last_flow_lpm;
  flow_index = (flow_index + 1) % FLOW_AVG_COUNT;

  float sum = 0.0f;
  for (int i = 0; i < FLOW_AVG_COUNT; i++) {
    sum += flow_history[i];
  }
  flowMeas.flow = sum / FLOW_AVG_COUNT;

  return flowMeas;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_10) {
    pulse_count++;
  }
}

size_t WatFlow_measure(uint8_t* data, SysTime_t ts, uint32_t idx) {
  // get timestamp
  SysTime_t diff = SysTimeSub(currentTime, lastTime);
  YFS210CMeasurement flowMeas = {};

  if (diff.SubSeconds >= 100) {  // If more than 0.1 seconds has passed
    flowMeas = FlowGetMeasurement();
  }

  /// read measurement
  flowMeas.flow = last_flow_lpm;
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  // encode measurement
  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  status = EncodeDoubleMeasurement(meta, flowMeas.flow, SensorType_YFS210C_FLOW,
                                   data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}
