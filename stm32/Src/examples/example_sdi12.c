/**
 * Copyright 2024 jLab
 * @file test_battery.c
 * @brief Prints out battery voltage levels
 *
 * In an infinite loop the battery voltage level is retrieved then outputted
 * over serial. The user should check if the voltage levels are expected. When
 * connected to USB the voltage should be ~5V. The battery voltage level should
 * be checked and compared to a multimeter measurement.
 *
 * @see battery.h
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-17
 */

#include <stdio.h>

#include "app_lorawan.h"
#include "gpio.h"
#include "lptim.h"
#include "main.h"
#include "rtc.h"
#include "sdi12.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "usart.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

void SystemClock_Config(void);

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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  // __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  // Add this back when the rtc is integrated.
  // Hopefully this will fix the issue.
  // Then you can remove the nops in SDI12_WakeSensors
  // MX_RTC_Init();

  /* Ensure that MSI is wake-up system clock */
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);

  /*Initialize timer and RTC*/
  UTIL_TIMER_Init();

  // HAL_TIM_OnePulse_MspInit(&htim1);
  // __HAL_RCC_TIM1_CLK_ENABLE();
  // HAL_TIM_OnePulse_Init(&htim1, TIM_OPMODE_SINGLE);

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
  SDI12_Measure_TypeDef measurment_info;

  // Infinite loop
  while (1) {
    // Print voltage level
    char buf[32];
    int buf_len = snprintf(buf, sizeof(buf), "0M!");

    if (SDI12GetMeasurment(addr, &measurment_info, buffer, 3000) == HAL_OK) {
      HAL_UART_Transmit(&huart1, (const uint8_t *)success, 7, 100);
      HAL_UART_Transmit(&huart1, buffer, 18, 100);
    } else {
      HAL_UART_Transmit(&huart1, (const uint8_t *)failure, 10, 100);
    }

    // Sleep
    for (int i = 0; i <= 1000000; i++) {
      asm("nop");
    }
    // HAL_Delay(DELAY);
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
   */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 42;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV5;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);
}
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /* USER CODE BEGIN Error_Handler_Debug */
  char error[30];
  int error_len =
      snprintf(error, sizeof(error), "Error!  HAL Status: %d\n", rc);
  HAL_UART_Transmit(&huart1, (const uint8_t *)error, error_len, 1000);

  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
