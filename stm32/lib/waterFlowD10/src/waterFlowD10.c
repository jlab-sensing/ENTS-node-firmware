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
extern volatile uint32_t pulse_count; // Managed by GPIO interrupt in stm32wlxx_it.c
static uint32_t previous_pulses = 0;
static SysTime_t irrigationStartTime;
static uint32_t irrigationStartPulseCount;
static bool irrigating;

static SysTime_t currentTime;
static SysTime_t lastTime;

void FlowInit() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOx_CLK_ENABLE(D10_GPIO_Port);

  // reset pins
  HAL_GPIO_WritePin(D10_GPIO_Port, D10_Pin, GPIO_PIN_RESET);

  // Configure PIN 9 on Port B (GPIO input)
  GPIO_InitStruct.Pin = D10_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(D10_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(D10_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(D10_EXTI_IRQn);

  // Get INIT Times
  previous_pulses = pulse_count;
  currentTime = SysTimeGet();
  lastTime = currentTime;
  irrigationStartTime = currentTime;
  irrigationStartPulseCount = previous_pulses;
  irrigating = false;
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
  returnValue.flow = (float)pulseDiff / ((diff.Seconds) / 60.0);
  previous_pulses = pulses;
  lastTime = currentTime;
  return returnValue;
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
