/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

// stdlib includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// system
#include "adc.h"
#include "app_lorawan.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "pb_encode.h"
#include "rtc.h"
#include "sdi12.h"
#include "sensor.h"
#include "stm32_systime.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "usart.h"

// user
#include "ads.h"

const int uart_timeout = 1000;

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  /*Initialize timer and RTC*/
  /*Have to be initilized in example files because LoRaWan cannot be initialized
   * like in main*/
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  UTIL_TIMER_Init();

  /* USER CODE BEGIN 2 */
  ADC_init();

  /* USER CODE BEGIN 2 */
  char controller_input[3];
  char check_input[7];
  char check_result[4];
  char size_proto_string[4];
  int size_check = snprintf(check_result, sizeof(check_result), "ok\n");
  // status for HAL_UART_* functions
  HAL_StatusTypeDef status = HAL_OK;

  // block for host to send check command
  status = HAL_UART_Receive(&huart1, (uint8_t *)check_input, 6, HAL_MAX_DELAY);
  if (status != HAL_OK) {
    Error_Handler();
  }

  // check for first chararcter in "check"
  if (check_input[0] == 'c') {
    status = HAL_UART_Transmit(
        &huart1, (uint8_t *)check_result, size_check,
        uart_timeout);  // send response to the 'check' command
    if (status != HAL_OK) {
      Error_Handler();
    }
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    // block until receive request for measurement
    status = HAL_UART_Receive(
        &huart1, (uint8_t *)controller_input, 1,
        HAL_MAX_DELAY);  // On every other iteration, send the encoded
                         // measurment in response to the '0' command
    if (status != HAL_OK) {
      continue;
    }

    // check command input
    if (controller_input[0] == '0') {
      SysTime_t ts = SysTimeGet();

      double voltage = ADC_readVoltage();
      double current = ADC_readCurrent();

      RepeatedSensorMeasurements measurements = {};
      measurements.measurements_count = 2;

      measurements.measurements[0].type = SensorType_POWER_VOLTAGE;
      measurements.measurements[0].which_value = SensorMeasurement_decimal_tag;
      measurements.measurements[0].value.decimal = voltage;

      measurements.measurements[1].type = SensorType_POWER_CURRENT;
      measurements.measurements[1].which_value = SensorMeasurement_decimal_tag;
      measurements.measurements[1].value.decimal = current;

      uint8_t encoded_measurement[256];
      pb_ostream_t ostream = pb_ostream_from_buffer(
          encoded_measurement, sizeof(encoded_measurement));
      bool status =
          pb_encode(&ostream, RepeatedSensorMeasurements_fields, &measurements);
      if (!status) {
        continue;
      }
      size_t measurement_size = ostream.bytes_written;

      // send length
      status = HAL_UART_Transmit(&huart1, (uint8_t *)&measurement_size, 1,
                                 uart_timeout);
      if (status != HAL_OK) {
        continue;
      }

      // send data
      status = HAL_UART_Transmit(&huart1, (uint8_t *)encoded_measurement,
                                 measurement_size, uart_timeout);
      if (status != HAL_OK) {
        continue;
      }
    }

    // delay between measurements
    HAL_Delay(100);
    /* USER CODE END 3 */
  }
}
