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
#include "sdi12.h"
#include "sys_app.h"
#include "waterFlow.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
  // MX_USART2_UART_Init();
  MX_I2C2_Init();

  /*Initialize timer and RTC*/
  /*Have to be initilized in example files because LoRaWan cannot be initialized
   * like in main*/
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  // UTIL_TIMER_Init();
  SystemApp_Init();
  UserConfigLoad();

  // TIMER_IF_Init();
  /* USER CODE BEGIN 2 */

  // Print the compilation time at startup
  char info_str[100];
  int info_len;
  info_len = snprintf(info_str, sizeof(info_str),
                      "Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n",
                      __DATE__, __TIME__);
  HAL_UART_Transmit(&huart1, (const uint8_t *)info_str, info_len, 1000);

  /* USER CODE BEGIN 2 */
  FlowInit();
  // FlowInit();
  //  TIMER_IF_Init();
  //  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  //  UTIL_TIMER_Init();

  char output[50];

  YFS210CMeasurement measurment;
  size_t reading_len;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    measurment = FlowGetMeasurment();
    reading_len = snprintf(output, sizeof(output), "Flow: %.4f \r\n",
                           measurment.flow);

    HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                      HAL_MAX_DELAY);

    for (int i = 0; i < 1000000; i++) {
      asm("nop");
    }
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */