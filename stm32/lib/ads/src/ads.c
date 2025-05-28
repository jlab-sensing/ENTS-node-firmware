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

#include "userConfig.h"

// #define CALIBRATION
#define DELTA_ENCODING
int32_t previous_voltage_reading = 0;
int32_t previous_current_reading = 0; 

// default sane calibration
#ifdef CALIBRATION
static double voltage_calibration_m = 0.0;
static double voltage_calibration_b = 0.0;
static double current_calibration_m = 0.0;
static double current_calibration_b = 0.0;
#endif /* CALIBRATION */
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
  const UserConfiguration* cfg = UserConfigGet();

  // read calibration values
  #ifdef CALIBRATION
  voltage_calibration_m = cfg->Voltage_Slope;
  voltage_calibration_b = cfg->Voltage_Offset;
  current_calibration_m = cfg->Current_Slope;
  current_calibration_b = cfg->Current_Offset;
  #endif /* CALIBRATION */

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
  previous_voltage_reading = 0; // set the previous values of delta encoding to 0
  previous_current_reading = 0;
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

int32_t ADC_twos_to_ones(int32_t delta){
  int32_t mask;
  int32_t abs_delta;
  int32_t ones_complement = 0xFFFFFFFF; // 32 bits of 1's
  if (delta < 0) { // check if the number is negative
    abs_delta = abs(delta);
    mask = (1 << 32) -1; // Create a mask for the given bit width (32)
    ones_complement = ~abs_delta & mask; // Invert the bits of the absolute value
  }
  return ones_complement;
}

int32_t ADC_readVoltage(void) {
  uint8_t code;
  int32_t reading;
  int32_t delta = 0;
  HAL_StatusTypeDef ret;
  uint8_t rx_data[3] = {0x00, 0x00, 0x00};  // ADC returns 3 bytes (24 bits)

  // Configure ADC for single-shot mode
  ret = ADC_configure(0x01);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }
  HAL_Delay(60);

  // Wait for the DRDY pin on the ADS12 to go low (data ready)
  while (HAL_GPIO_ReadPin(data_ready_port, data_ready_pin)) {
  }

  // Request ADC data
  code = ADS12_READ_DATA_CODE;
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1, HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }
  ret = HAL_I2C_Master_Receive(&hi2c2, ADS12_READ, rx_data, 3, 1000);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }

  // Combine the 3 bytes into a 24-bit signed value
  int32_t temp = ((int32_t)rx_data[0] << 16) | ((int32_t)rx_data[1] << 8) |
                 ((int32_t)rx_data[2]);
  // Extend the sign bit if the 24th bit is set
  if (temp & 0x800000) {
    temp |= 0xFF000000;
  }
  reading = temp;

#ifdef CALIBRATION
  // Apply calibration if enabled
  reading = (int32_t)((voltage_calibration_m * reading) + voltage_calibration_b);
#endif /* CALIBRATION */

#ifdef DELTA_ENCODING
  // Calculate delta and convert to 1's complement
  delta = reading - previous_voltage_reading;  // Calculate the delta
  previous_voltage_reading = reading;         // Update the previous reading
  reading = ADC_twos_to_ones(delta);          // Convert to 1's complement
#endif /* DELTA_ENCODING */

  return reading;
}

int32_t ADC_readCurrent(void) {
  uint8_t code;
  int32_t reading;
  int32_t delta = 0;
  HAL_StatusTypeDef ret;
  uint8_t rx_data[3] = {0x00, 0x00, 0x00};  // ADC returns 3 bytes (24 bits)

  // Configure ADC for single-shot mode to read current
  ret = ADC_configure(0x21);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }
  HAL_Delay(60);

  // Wait for the DRDY pin on the ADS12 to go low (data ready)
  while (HAL_GPIO_ReadPin(data_ready_port, data_ready_pin)) {
  }

  // Request ADC data
  code = ADS12_READ_DATA_CODE;
  ret = HAL_I2C_Master_Transmit(&hi2c2, ADS12_WRITE, &code, 1, HAL_MAX_DELAY);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }
  ret = HAL_I2C_Master_Receive(&hi2c2, ADS12_READ, rx_data, 3, 1000);
  if (ret != HAL_OK) {
    return -1;  // Return -1 on error
  }

  // Combine the 3 bytes into a 24-bit signed value
  int32_t temp = ((int32_t)rx_data[0] << 16) | ((int32_t)rx_data[1] << 8) |
                 ((int32_t)rx_data[2]);
  // Extend the sign bit if the 24th bit is set
  if (temp & 0x800000) {
    temp |= 0xFF000000;
  }
  reading = temp;

#ifdef CALIBRATION
  // Apply calibration if enabled
  reading = (int32_t)((current_calibration_m * reading) + current_calibration_b);
#endif /* CALIBRATION */

#ifdef DELTA_ENCODING
  // Calculate delta and convert to 1's complement
  delta = reading - previous_current_reading;  // Calculate the delta
  previous_current_reading = reading;         // Update the previous reading
  reading = ADC_twos_to_ones(delta);          // Convert to 1's complement
#endif /* DELTA_ENCODING */

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
  int32_t adc_voltage = ADC_readVoltage();
  int32_t adc_current = ADC_readCurrent();

  const UserConfiguration* cfg = UserConfigGet();

  // encode measurement
  #ifndef DELTA_ENCODING
  size_t data_len = EncodePowerMeasurement(
      ts.Seconds, cfg->logger_id, cfg->cell_id, adc_voltage, adc_current, data);
  #else
  size_t data_len = EncodePowerDeltaMeasurement(
      ts.Seconds, cfg->logger_id, cfg->cell_id, (uint32_t)adc_voltage, (uint32_t)adc_current, data);
  #endif /* DELTA_ENCODING */

  // return number of bytes in serialized measurement
  return data_len;
}
