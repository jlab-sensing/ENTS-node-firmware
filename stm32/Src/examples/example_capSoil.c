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
#include "adc.h"
#include "app_lorawan.h"
#include "board.h"
#include "gpio.h"
#include "main.h"
#include "rtc.h"
#include "sen0308.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "usart.h"

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
  MX_USART2_UART_Init();
  // MX_LPUART1_UART_Init();
  MX_I2C1_Init();

  SystemApp_Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS,

          "Soil Power Sensor Wio-E5 firmware, test: %s, compiled on %s %s\n",
          __FILE__, __DATE__, __TIME__);

  UserConfiguration cfg = UserConfiguration_init_zero;
  cfg.enabled_sensors_multiple_count = 1;         // not used in this example
  cfg.enabled_sensors_multiple[0].cell_id = 200;  // not used in this example
  cfg.enabled_sensors_multiple[0].enabled_sensor = EnabledSensor_SEN0308;
  cfg.enabled_sensors_multiple[0].index = 21;  // ADC channel 1, pin 21
  CapSoilInit(&(cfg.enabled_sensors_multiple[0]));

  SEN0308Measurement measurement;

  while (1) {
    measurement = SEN0308GetMeasurement(&(cfg.enabled_sensors_multiple[0]));
    APP_LOG(TS_OFF, VLEVEL_ALWAYS,
            "Voltage: %.4f V\r\nSoil Humidity: %.3f%%\r\n", measurement.voltage,
            measurement.humidity);

    HAL_Delay(1000);
  }
}
