/**
 ******************************************************************************
 * @file    waterFlowYFS210C.c
 * @author  Caden Jacobs
 *
 * @brief   This library is designed to read measurements from a Water Flow
 *Sensor
 *          https://www.danomsk.ru/upload/iblock/43d/193917_3b664efb7b37f7ae8ea1eea40978a265.pdf
 *
 * @date    7/31/2025
 ******************************************************************************
 */

#include "waterFlowYFS210C.h"

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
extern volatile uint32_t
    pulse_count;  // Managed by GPIO interrupt in stm32wlxx_it.c
static volatile float last_flow_lpm = 0;
static SysTime_t currentTime;
static SysTime_t lastTime;
static volatile float flow_history[FLOW_AVG_COUNT] = {0};
static uint8_t flow_index = 0;

// For every one liter of water that passes through the sensor in one minute,
// there are 450 pulses. Therefore the calibration factor becomes [450/60 = 7.5]
const float calibration_factor = 7.5;

void FlowYFS210CInit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOx_CLK_ENABLE(YFS201C_GPIO_Port);

  // reset pins
  HAL_GPIO_WritePin(YFS201C_GPIO_Port, YFS201C_Pin, GPIO_PIN_RESET);

  // Configure PIN 9 on Port B (GPIO input)
  GPIO_InitStruct.Pin = YFS201C_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(YFS201C_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(YFS201C_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(YFS201C_EXTI_IRQn);

  // Get INIT Times
  currentTime = SysTimeGet();
  lastTime = currentTime;
}

YFS210CMeasurement FlowYFS210CGetMeasurement() {
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

size_t WatFlowYFS210C_measure(uint8_t* data, SysTime_t ts, uint32_t idx,
                              EnabledSensorMultiple* sensor) {
  // get timestamp
  SysTime_t diff = SysTimeSub(currentTime, lastTime);
  YFS210CMeasurement flowMeas = {};

  if (diff.SubSeconds >= 100) {  // If more than 0.1 seconds has passed
    flowMeas = FlowYFS210CGetMeasurement();
  }

  /// read measurement
  flowMeas.flow = last_flow_lpm;
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

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
