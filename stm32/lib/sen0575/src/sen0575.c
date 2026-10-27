/**
 ******************************************************************************
 * @file    sen0575.c
 * @author  Eric Tran
 *
 * @brief   This library is designed to read data from the sen0575
 *          rainfall sensor
 *
 * @date    10/6/2026
 ******************************************************************************
 */

#include "sen0575.h"

// user includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sensor.h"
#include "sensors.h"
#include "stm32wlxx_hal_def.h"
#include "transcoder.h"
#include "usart.h"
#include "userConfig.h"

#define NO_RAIN 0
#define RECORDING_PERIOD 86400
#define COUNT_PER_TIP 3  // currently somewhat varies and needs debouncing
#define ML_PER_TIP 0.4

/*Private function prototypes*/
// unsigned char I2C_ReadRegister(unsigned char I2CAddress,
//                                unsigned char deviceRegisterAddress);
// unsigned char I2C_ChannelReadRegister(unsigned char I2CAddress,
//                                       unsigned char deviceRegisterAddress);
// unsigned char I2C_WriteReg(unsigned char I2CAddress,
//                            unsigned char deviceRegisterAddress, uint8_t
//                            data);

// Variables
extern volatile uint32_t
    pulse_count;  // Managed by GPIO interrupt in stm32wlxx_it.c
static uint32_t start_pulses = 0;
static SysTime_t rainStartTime;
static SysTime_t lastRainTime;
static uint32_t rainStartPulseCount;
static bool raining;

static SysTime_t currentTime;
static SysTime_t lastTime;

void sen0575Init() {
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOx_CLK_ENABLE(SEN0575_GPIO_Port);

  // reset pins
  HAL_GPIO_WritePin(SEN0575_GPIO_Port, SEN0575_Pin, GPIO_PIN_RESET);

  // Configure PIN 15 on Port A (GPIO input)
  GPIO_InitStruct.Pin = SEN0575_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(SEN0575_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(SEN0575_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SEN0575_EXTI_IRQn);

  // Get INIT Times
  start_pulses = pulse_count;
  currentTime = SysTimeGet();
  lastTime = currentTime;
  rainStartTime = currentTime;
  lastRainTime = currentTime;
  rainStartPulseCount = start_pulses;
  raining = false;
}

SEN0575Measurement sen0575RainfallRaw() {
  // get time
  currentTime = SysTimeGet();
  SysTime_t diff = SysTimeSub(currentTime, lastTime);
  // Always calculate flow, not just every 100ms
  uint16_t pulses = pulse_count;
  uint16_t pulseDiff = pulses - start_pulses;
  static uint32_t lastPulseDiff = 0;
  SEN0575Measurement returnValue = {0};

  // used for debouncing
  static uint16_t tipCount = 0;

  // no rainfall occuring, moving start time and pulse count forward to catch
  // rainfall
  if (pulseDiff == NO_RAIN && !raining) {
    rainStartPulseCount = pulses;
    rainStartTime = currentTime;
    start_pulses = pulses;
  }  // ending of recording period
  else if ((currentTime.Seconds - rainStartTime.Seconds >= RECORDING_PERIOD)) {
    // reseting the recording period
    raining = false;
    tipCount = 0;
    rainStartPulseCount = pulses;
    rainStartTime =
        currentTime;  // if not currently raining then set start time to now
    start_pulses = pulses;
  }
  // beginning of rainfall period (rising edge detected) and continued
  // rainfall phase pulses have increased from last checked period
  else {
    // submit pulses since start time and last rain time
    raining = true;
    returnValue.volumeElapsed = pulses - rainStartPulseCount;

    if ((pulseDiff != lastPulseDiff) && (pulseDiff > 0)) {
      lastRainTime = currentTime;  // time since last rain
      tipCount += 1;
    }
  }

  // calculating rainfall per recording period (24 hours)
  returnValue.volumeElapsed =
      (((float)tipCount / COUNT_PER_TIP) *
       ML_PER_TIP);  // average of 3 count per tip, 1 tip is around 0.4 ml

  returnValue.lastRainTime = currentTime.Seconds - lastRainTime.Seconds;
  lastPulseDiff = pulseDiff;
  return returnValue;
}

size_t sen0575_measure(uint8_t* data, SysTime_t ts, uint32_t idx,
                       EnabledSensorMultiple* sensor) {
  // get timestamp
  SEN0575Measurement rainMeas = {};

  rainMeas = sen0575RainfallRaw();

  /// read measurement
  const UserConfiguration* cfg = UserConfigGet();

  // metadata
  Metadata meta = Metadata_init_zero;
  meta.ts = ts.Seconds;
  meta.logger_id = cfg->logger_id;
  meta.cell_id = sensor->cell_id;

  // encode measurement
  size_t data_len = 0;
  SensorStatus status = SENSOR_OK;

  SensorsAddMeasurement(data, data_len);
  status = EncodeUint32Measurement(meta, rainMeas.volumeElapsed,
                                   SensorType_SEN0575_VOLUME_ELAPSED, data,
                                   &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }
  SensorsAddMeasurement(data, data_len);

  status = EncodeUint32Measurement(meta, rainMeas.lastRainTime,
                                   SensorType_SEN0575_LAST_RAIN_TIME, data,
                                   &data_len);
  if (status != SENSOR_OK) {
    return -1;
  }

  // return number of bytes in serialized measurement
  return data_len;
}

////////////////////////////////////////////

/*unused code for i2c module*/
// uint32_t sen0575RainfallCumulative(void)
// {
//   uint8_t data[4];

//   HAL_StatusTypeDef ret;
//   uint8_t registerAddress = SEN0575_I2C_REG_CUMULATIVE_RAINFALL;

//   // start condition
//   ret = HAL_I2C_Master_Transmit(
//   &hi2c1, SEN0575_DEVICE_ADDRESS << 1, &registerAddress, 1,
//   HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
//   }

//   // get byte
//   ret = HAL_I2C_Master_Receive(
//   &hi2c1, SEN0575_DEVICE_ADDRESS << 1, (uint8_t *)data,
//   4, // gets 4 bytes
//   HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
//   if (ret != HAL_OK) {
//       APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
//   }

//   uint32_t cumulativeRainfall = ((uint32_t)data[0] |
//   (uint32_t)data[1] << 8 |
//   (uint32_t)data[2] << 16 |
//   (uint32_t)data[3] << 24);

//   return cumulativeRainfall;
// }

// uint32_t sen0575RainfallRaw(void)
// {
//     uint8_t data[4];

//     HAL_StatusTypeDef ret;
//     uint8_t registerAddress = SEN0575_I2C_REG_RAW_DATA;

//     // start condition
//     ret = HAL_I2C_Master_Transmit(
//     &hi2c1, SEN0575_DEVICE_ADDRESS << 1, &registerAddress, 1,
//     HAL_MAX_DELAY);  // wait for it to end, this is internal and can not
//     stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
//   }

//   // get byte
//     ret = HAL_I2C_Master_Receive(
//     &hi2c1, SEN0575_DEVICE_ADDRESS << 1, (uint8_t *)data,
//     4, // gets 4 bytes
//     HAL_MAX_DELAY);  // wait for it to end, this is internal and can not
//     stall if (ret != HAL_OK) {
//         APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
//     }

//     uint32_t returnedRawData = ((uint32_t)data[0] |
//     (uint32_t)data[1] << 8 |
//     (uint32_t)data[2] << 16 |
//     (uint32_t)data[3] << 24);

//     return returnedRawData;
// }

/*Private Functions*/

// unsigned char I2C_ReadRegister(unsigned char I2CAddress,
//                                unsigned char deviceRegisterAddress) {
//   HAL_StatusTypeDef ret;

//   I2CAddress = I2CAddress << 1;  // use 8-bit address
//   uint8_t *data = &deviceRegisterAddress;

//   // start condition
//   ret = HAL_I2C_Master_Transmit(
//       &hi2c1, I2CAddress, data, 1,
//       HAL_MAX_DELAY);  // wait for it to end, this is internal and can not
//       stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
//     return ERROR;
//   }

//   // get byte
//   ret = HAL_I2C_Master_Receive(
//       &hi2c1, I2CAddress, data, 1,
//       HAL_MAX_DELAY);  // wait for it to end, this is internal and can not
//       stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
//     return ERROR;
//   }

//   return *data;
// }

// unsigned char I2C_ChannelReadRegister(unsigned char I2CAddress,
//                                       unsigned char deviceRegisterAddress) {
//   HAL_StatusTypeDef ret;

//   I2CAddress = I2CAddress << 1;  // use 8-bit address
//   uint8_t *data = &deviceRegisterAddress;

//   // start condition
//   ret = HAL_I2C_Master_Transmit(
//       &hi2c1, I2CAddress, data, 1,
//       HAL_MAX_DELAY);  // wait for it to end, this is internal and can not
//       stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
//     return ERROR;
//   }

//   // get byte
//   ret = HAL_I2C_Master_Receive(
//       &hi2c1, I2CAddress, data, 4,
//       HAL_MAX_DELAY);  // wait for it to end, this is internal and can not
//       stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
//     return ERROR;
//   }

//   return *data;
// }

// unsigned char I2C_WriteReg(unsigned char I2CAddress,
//                            unsigned char deviceRegisterAddress, uint8_t data)
//                            {
//   HAL_StatusTypeDef ret;
//   I2CAddress = I2CAddress << 1;  // use 8-bit address

//   ret = HAL_I2C_Mem_Write(&hi2c1, I2CAddress, deviceRegisterAddress,
//                           I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on write data\r\n");
//     return ERROR;
//   }

//   return SUCCESS;
// }
