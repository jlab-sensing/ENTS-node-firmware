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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

// system includes
#include "app_lorawan.h"
#include "board.h"
#include "gpio.h"
#include "lptim.h"
#include "main.h"
#include "rtc.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "usart.h"
#include "ads.h"
#include "sen0308.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

/** Global variable for all return codes */
HAL_StatusTypeDef rc;

/**
 * @brief Entry point for battery test
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals,
  Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();

  SystemApp_Init();
  UserConfigLoad();

  // User level initialization

  // Print the compilation time at startup
  char info_str[128];
  int info_len;
  info_len = snprintf(
      info_str, sizeof(info_str),
      "Soil Power Sensor Wio-E5 firmware, test: %s, compiled on %s %s\n",
      __FILE__, __DATE__, __TIME__);
  HAL_UART_Transmit(&huart1, (const uint8_t *)info_str, info_len, 1000);
  char success[] = "HAL_OK\n";
  char failure[] = "HAL_FAIL\n";
  char buffer[20];
  uint8_t addr = '0';

  CapSoilInit();
  //ADC_init();
  char output[35];
  
  
  SEN0308_measurments measurment;
  size_t reading_len;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    measurment = SEN0308GetMeasurment();
    reading_len = snprintf(output, sizeof(output), "Soil Humidity: %.3f%%\r\n", measurment.capSoil_calibrated);
    HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                      HAL_MAX_DELAY);
    // for (int i = 0; i < 10000; i++){
    //   asm("nop");
    // }

    //HAL_Delay(
    //    1000);  // I guess HAL_Delay is broken somehow, don't understand why
    for (int i = 0; i < 1000000; i++){
     asm("nop");
    }
  }
}

