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
#include "sensor.h"
#include "sensors.h"
#include "transcoder.h"
#include "userConfig.h"



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32wlxx_hal_def.h"

/*Private function prototypes*/
unsigned char I2C_ReadRegister(unsigned char I2CAddress,
                               unsigned char deviceRegisterAddress);
unsigned char I2C_ChannelReadRegister(unsigned char I2CAddress,
                                      unsigned char deviceRegisterAddress);
unsigned char I2C_WriteReg(unsigned char I2CAddress,
                           unsigned char deviceRegisterAddress, uint8_t data);

////////////////////////////////////////////
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
extern volatile uint32_t
    pulse_count;  // Managed by GPIO interrupt in stm32wlxx_it.c
static uint32_t previous_pulses = 0;
static SysTime_t irrigationStartTime;
static uint32_t irrigationStartPulseCount;
static bool irrigating;

static SysTime_t currentTime;
static SysTime_t lastTime;

void sen0575Init() {
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
  HAL_NVIC_SetPriority(SEN0575_EXTI_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SEN0575_EXTI_IRQn);

  // Get INIT Times
  previous_pulses = pulse_count;
  currentTime = SysTimeGet();
  lastTime = currentTime;
  irrigationStartTime = currentTime;
  irrigationStartPulseCount = previous_pulses;
  irrigating = false;
}

D10Measurement sen0575RainfallRaw() {
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

size_t sen0575_measure(uint8_t* data, SysTime_t ts, uint32_t idx,
                          EnabledSensorMultiple* sensor) {
  // get timestamp
  D10Measurement flowMeas = {};

  flowMeas = sen0575RainfallRaw();

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

////////////////////////////////////////////
// HAL_StatusTypeDef sen0575Init(void)
// {

// }


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
//     HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
//   if (ret != HAL_OK) {
//     APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
//   }

//   // get byte
//     ret = HAL_I2C_Master_Receive(
//     &hi2c1, SEN0575_DEVICE_ADDRESS << 1, (uint8_t *)data,
//     4, // gets 4 bytes
//     HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
//     if (ret != HAL_OK) {
//         APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
//     }

//     uint32_t returnedRawData = ((uint32_t)data[0] | 
//     (uint32_t)data[1] << 8 |
//     (uint32_t)data[2] << 16 |
//     (uint32_t)data[3] << 24); 

//     return returnedRawData;
// }
/////////////////////////////////////////////////////////////////////////
/*Private Functions*/

unsigned char I2C_ReadRegister(unsigned char I2CAddress,
                               unsigned char deviceRegisterAddress) {
  HAL_StatusTypeDef ret;

  I2CAddress = I2CAddress << 1;  // use 8-bit address
  uint8_t *data = &deviceRegisterAddress;

  // start condition
  ret = HAL_I2C_Master_Transmit(
      &hi2c1, I2CAddress, data, 1,
      HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
  if (ret != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
    return ERROR;
  }

  // get byte
  ret = HAL_I2C_Master_Receive(
      &hi2c1, I2CAddress, data, 1,
      HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
  if (ret != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
    return ERROR;
  }

  return *data;
}

unsigned char I2C_ChannelReadRegister(unsigned char I2CAddress,
                                      unsigned char deviceRegisterAddress) {
  HAL_StatusTypeDef ret;

  I2CAddress = I2CAddress << 1;  // use 8-bit address
  uint8_t *data = &deviceRegisterAddress;

  // start condition
  ret = HAL_I2C_Master_Transmit(
      &hi2c1, I2CAddress, data, 1,
      HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
  if (ret != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
    return ERROR;
  }

  // get byte
  ret = HAL_I2C_Master_Receive(
      &hi2c1, I2CAddress, data, 4,
      HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
  if (ret != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
    return ERROR;
  }

  return *data;
}

unsigned char I2C_WriteReg(unsigned char I2CAddress,
                           unsigned char deviceRegisterAddress, uint8_t data) {
  HAL_StatusTypeDef ret;
  I2CAddress = I2CAddress << 1;  // use 8-bit address

  ret = HAL_I2C_Mem_Write(&hi2c1, I2CAddress, deviceRegisterAddress,
                          I2C_MEMADD_SIZE_8BIT, &data, 1, HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on write data\r\n");
    return ERROR;
  }

  return SUCCESS;
}







// void SolenoidInit() {
//   __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA clock

//   // Use Pin PA10 to toggle solenoid
//   GPIO_InitTypeDef GPIO_InitStruct = {0};
//   GPIO_InitStruct.Pin = GPIO_PIN_10;
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Push-pull output
//   GPIO_InitStruct.Pull = GPIO_NOPULL;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

//   // Set up interrupt
//   HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//   // Init state of Solenoid
//   SolenoidClose();
//   Solenoid = SOLENOID_OFF;
// }

// void SolenoidOpen(void) {
//   printf("SOLENOID_OPEN: Setting PA10 to HIGH (Relay ON)\n");
//   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);  // HIGH = Relay ON
//   Solenoid = SOLENOID_ON;
// }

// void SolenoidClose(void) {
//   printf("SOLENOID_CLOSE: Setting PA10 to LOW (Relay OFF)\n");
//   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay OFF
//   Solenoid = SOLENOID_OFF;
// }
