/**
 ******************************************************************************
 * @file    ads.c
 * @author  Stephen Taylor
 * @brief   Soil Power Sensor ADS12 library
 *          This file provides a function to read from the onboard ADC
 *(ADS1219).
 * @date    11/27/2023
 *
 ******************************************************************************
 **/

/* Includes ------------------------------------------------------------------*/
#include "ads.h"

#include <stm32wlxx_hal_gpio.h>

// #define CALIBRATION

const double voltage_calibration_m = -0.00039346;
const double voltage_calibration_b = -0.5442246923562877;
const double current_calibration_m = -1.187971e-10;
const double current_calibration_b = 3.9018992737640816e-05;

/**
 * @brief GPIO port for adc data ready line
 *
 * @see data_ready_pin
 */
const GPIO_TypeDef* data_ready_port = GPIOC;

/**
 * @brief GPIO pin for adc data ready line
 *
 */
const uint16_t data_ready_pin = GPIO_PIN_0;

int HAL_status(HAL_StatusTypeDef ret) {
  int status;
  if (ret == HAL_OK) {
    status = 0;
  } else if (ret == HAL_ERROR) {
    status = 1;
  } else if (ret == HAL_BUSY) {
    status = 2;
  } else {
    status = 3;
  }
  return status;
}

HAL_StatusTypeDef ADC_init(void) {
  uint8_t code = ADS12_RESET_CODE;
  uint8_t register_data[2] = {0x40, 0x03};
  HAL_StatusTypeDef ret;

  // Control register breakdown.
  //  7:5 MUX (default)
  //  4   Gain (default)
  //  3:2 Data rate (default)
  //  1   Conversion mode (default)
  //  0   VREF (External reference 3.3V)

  HAL_GPIO_WritePin(
      GPIOA, GPIO_PIN_9,
      GPIO_PIN_SET);  // Power down pin has to be set to high before any of the
                      // analog circuitry can function
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1,
                                HAL_MAX_DELAY);  // Send the reset code
  if (ret != HAL_OK) {
    return ret;
  }

  // Set the control register, leaving everything at default except for the
  // VREF, which will be set to external reference mode
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, register_data, 2,
                                HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    return ret;
  }

  code = ADS12_START_CODE;
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1,
                                HAL_MAX_DELAY);  // Send a start code
  if (ret != HAL_OK) {
    return ret;
  }
  HAL_Delay(500);  // Delay to allow ADC start up, not really sure why this is
                   // neccesary, or why the minimum is 300
  return ret;
}

HAL_StatusTypeDef ADC_configure(uint8_t reg_data) {
  uint8_t code = ADS12_RESET_CODE;
  uint8_t register_data[2] = {0x40, reg_data};
  HAL_StatusTypeDef ret;
  uint8_t recx_reg;
  char reg_string[40];

  // Set the control register, leaving everything at default except for the
  // VREF, which will be set to external reference mode
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, register_data, 2,
                                HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    return ret;
  }

  // Send a start code
  code = ADS12_START_CODE;
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1, HAL_MAX_DELAY);
  return ret;
}

double ADC_readVoltage(void) {
  uint8_t code;
  double reading;
  HAL_StatusTypeDef ret;
  uint8_t rx_data[3] = {0x00, 0x00, 0x00};  // Why is this only 3 bytes?

  // 0x01 is single shot 0x03 is continuous
  ret = ADC_configure(0x01);
  if (ret != HAL_OK) {
    return -1;
  }
  HAL_Delay(60);
  /*
  // Modification
  uint8_t coderreg = 0b00100000; // read from configuration register 0
  uint8_t rx_datarreg[1] = {0x00}; // rreg only returns 1 byte of data
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &coderreg, 1,
  HAL_MAX_DELAY); if (ret != HAL_OK){ return -1;
  }
  ret = HAL_I2C_Master_Receive(&hi2c2, ADS12_READ, rx_datarreg, 1, 1000);
  if (ret != HAL_OK){
    return -1;
  }
  // end of modification
  */

  // Wait for the DRDY pin on the ADS12 to go low, this means data is ready
  while (HAL_GPIO_ReadPin(data_ready_port, data_ready_pin)) {
  }

  code = ADS12_READ_DATA_CODE;
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1, HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    return -1;
  }
  ret = HAL_I2C_Master_Receive(&hi2c2, ADS12_READ, rx_data, 3, 1000);
  if (ret != HAL_OK) {
    return -1;
  }

  // Combine the 3 bytes into a 24-bit value
  int32_t temp = ((int32_t)rx_data[0] << 16) | ((int32_t)rx_data[1] << 8) |
                 ((int32_t)rx_data[2]);
  // Check if the sign bit (24th bit) is set
  if (temp & 0x800000) {
    // Extend the sign to 32 bits
    temp |= 0xFF000000;
  }
  reading = (double)temp;

#ifndef CALIBRATION
  reading = (voltage_calibration_m * reading) + voltage_calibration_b;
#endif /* CALIBRATION */

  // tmp fix: convert mV to volts
  reading /= 1000.0;

  return reading;
}

double ADC_readCurrent(void) {
  uint8_t code;
  double reading;
  HAL_StatusTypeDef ret;
  uint8_t rx_data[3] = {0x00, 0x00, 0x00};

  // 0x21 is single shot and 0x23 is continuos
  ret = ADC_configure(0x21);  // configure to read current
  if (ret != HAL_OK) {
    return -1;
  }
  HAL_Delay(60);
  /*
  // Modification
  uint8_t coderreg = 0b00100000; // read from configuration register 0
  uint8_t rx_datarreg[1] = {0x00}; // rreg only returns 1 byte of data
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &coderreg, 1,
  HAL_MAX_DELAY); if (ret != HAL_OK){ return -1;
  }
  ret = HAL_I2C_Master_Receive(&hi2c2, ADS12_READ, rx_datarreg, 1, 1000);
  if (ret != HAL_OK){
    return -1;
  }
  // end of modification
  */

  // Wait for the DRDY pin on the ADS12 to go low, this means data is ready
  while (HAL_GPIO_ReadPin(data_ready_port, data_ready_pin)) {
  }
  code = ADS12_READ_DATA_CODE;
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1, HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    return -1;
  }
  ret = HAL_I2C_Master_Receive(&hi2c2, ADS12_READ, rx_data, 3, 1000);
  if (ret != HAL_OK) {
    return -1;
  }

  // Combine the 3 bytes into a 24-bit value
  int32_t temp = ((int32_t)rx_data[0] << 16) | ((int32_t)rx_data[1] << 8) |
                 ((int32_t)rx_data[2]);
  // Check if the sign bit (24th bit) is set
  if (temp & 0x800000) {
    // Extend the sign to 32 bits
    temp |= 0xFF000000;
  }
  reading = (double)temp;

#ifndef CALIBRATION
  reading = (current_calibration_m * reading) +
            current_calibration_b;  // Calculated from linear regression
#endif                              /* CALIBRATION */

  return reading;
}

HAL_StatusTypeDef probeADS12(void) {
  HAL_StatusTypeDef ret;
  ret = HAL_I2C_IsDeviceReady(&hi2c2, ADS12_WRITE, 10, 20);
  return ret;
}

size_t ADC_measure(uint8_t* data) {
  // get timestamp
  SysTime_t ts = SysTimeGet();

  // read power
  double adc_voltage = ADC_readVoltage();
  double adc_current = ADC_readCurrent();

  // encode measurement
  size_t data_len = EncodePowerMeasurement(ts.Seconds, LOGGER_ID, CELL_ID,
                                           adc_voltage, adc_current, data);

  // return number of bytes in serialized measurement
  return data_len;
}
