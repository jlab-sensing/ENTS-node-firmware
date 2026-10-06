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

SEN0257Measurement sen0575WorkingTime(void)
{

}

SEN0257Measurement sen0575RainfallCumulative(void)
{

}

uint32_t sen0575RainfallRaw(void)
{
    uint8_t data[4];

    HAL_StatusTypeDef ret;
    uint8_t registerAddress = SEN0575_I2C_REG_RAW_DATA;
    
    // start condition
    ret = HAL_I2C_Master_Transmit(
    &hi2c1, SEN0575_DEVICE_ADDRESS << 1, &registerAddress, 1,
    HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
  if (ret != HAL_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "I2C Tx Error on read start condition\r\n");
  }

  // get byte
    ret = HAL_I2C_Master_Receive(
    &hi2c1, SEN0575_DEVICE_ADDRESS << 1, (uint8_t *)data,
    4, // gets 4 bytes
    HAL_MAX_DELAY);  // wait for it to end, this is internal and can not stall
    if (ret != HAL_OK) {
        APP_LOG(TS_OFF, VLEVEL_M, "I2C Rx Error on read byte\r\n");
    }

    uint32_t returnedRawData = ((uint32_t)data[0] | 
    (uint32_t)data[1] << 8 |
    (uint32_t)data[2] << 16 |
    (uint32_t)data[3] << 24); 

    return returnedRawData;
}

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
