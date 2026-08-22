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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "app_lorawan.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "ads.h"
#include "rtc.h"
#include "sys_app.h"
#include "waterPressure.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
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
  UserConfigLoad();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS,
          "Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n", __DATE__,
          __TIME__);

  UserConfiguration cfg = UserConfiguration_init_zero;
  cfg.enabled_sensors_multiple_count = 1;         // not used in this example
  cfg.enabled_sensors_multiple[0].cell_id = 200;  // not used in this example
  cfg.enabled_sensors_multiple[0].enabled_sensor = EnabledSensor_SEN0257;
  cfg.enabled_sensors_multiple[0].index = 21;  // ADC channel 1, pin 21
  PressureInit(&(cfg.enabled_sensors_multiple[0]));

  SEN0257Measurement measurement;
  size_t reading_len;

  while (1) {
    measurement = PressureGetMeasurement(&(cfg.enabled_sensors_multiple[0]));
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Voltage: %.4f V\r\nPressure: %.4f kPa\r\n",
            measurement.voltage, measurement.pressure);

    HAL_Delay(1000);
  }
}
