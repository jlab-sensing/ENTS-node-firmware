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

#include "waterFlowD10.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sensor.h"
#include "sensors.h"
#include "stm32wlxx_hal_def.h"
#include "transcoder.h"
#include "usart.h"
#include "userConfig.h"

// may need to change depending on specifics of irrigation system
#define NO_IRRIGATION_FLOW_G 1

// Variables
static volatile unsigned long pulse_count = 0;
static uint32_t previous_pulses = 0;
static SysTime_t irrigationStartTime;
static uint32_t irrigationStartPulseCount;
static bool irrigating;

SysTime_t currentTime;
SysTime_t lastTime;

void FlowInit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  // Configure PA10 as rising edge EXTI input (water flow sensor output)
  __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable  clock for port A

  // reset pins
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

  // Configure PIN 10 on Port A (GPIO input)
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // interrupt on falling edge
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  // Get INIT Times
  previous_pulses = pulse_count;
  currentTime = SysTimeGet();
  lastTime = currentTime;
  irrigationStartTime = currentTime;
  irrigationStartPulseCount = previous_pulses;
}

D10Measurement FlowGetMeasurement() {
  // get time
  currentTime = SysTimeGet();
  SysTime_t diff = SysTimeSub(currentTime, lastTime);
  // Always calculate flow, not just every 100ms
  uint32_t pulses = pulse_count;
  uint32_t pulseDiff = pulses - previous_pulses;
  D10Measurement returnValue = {0};

  // no irrigation occuring, moving start time and pulse count forward to catch
  // irrigation
  if (pulseDiff == NO_IRRIGATION_FLOW_G && !irrigating) {
    irrigationStartPulseCount = pulses;
    irrigationStartTime = currentTime;
  }  // ending of irrigation phase (falling edge detected)
  else if (pulseDiff == NO_IRRIGATION_FLOW_G) {
    // need to calculate and submit elapsed time
    // submit currentTime - irrigationStartTime
    irrigating = false;
    returnValue.timeElapsed = currentTime.Seconds - irrigationStartTime.Seconds;
  }
  // beginning of irrigation phase (rising edge detected) and continued
  // irrgating phase pulses have increased from last checked period
  else {
    // submit pulses - irrigationStartPulseCount
    irrigating = true;
    returnValue.volumeElapsed = pulses - irrigationStartPulseCount;
  }

  // calculation of flow rate
  float flowRateGPM = pulseDiff / ((diff.Seconds) / 60);
  returnValue.flow = flowRateGPM;
  previous_pulses = pulses;
  lastTime = currentTime;
  return returnValue;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
  if (GPIO_Pin == GPIO_PIN_10) {
    pulse_count++;
  }
}

size_t WatFlow_measure(uint8_t* data, SysTime_t ts, uint32_t idx) {
  // get timestamp
  D10Measurement flowMeas = {};

  flowMeas = FlowGetMeasurement();

  /// read measurement
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = cfg->cell_id;

  // encode measurement
  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  status = EncodeDoubleMeasurement(meta, flowMeas.flow, SensorType_D10_FLOW,
                                   data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  SensorsAddMeasurement(data, data_len);
  status =
      EncodeUint32Measurement(meta, flowMeas.volumeElapsed,
                              SensorType_D10_VOLUME_ELAPSED, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  status = EncodeUint32Measurement(
      meta, flowMeas.timeElapsed, SensorType_D10_TIME_ELAPSED, data, &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}
