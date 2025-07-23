/**
 * @example example_teros12.c
 *
 * Prints teros12 measurements over serial.
 *
 * @author John Madden
 * @date 2025-02-11
 */

#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "app_lorawan.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "sys_app.h"
#include "teros12.h"
#include "usart.h"

void SystemClock_Config(void);

/* USER CODE END 0 */
/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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

  // Print the compilation time at startup
  APP_LOG(
      TS_OFF, VLEVEL_M,
      "Utility to change address of SDI-12 based sensor, compiled on %s %s\r\n",
      __DATE__, __TIME__);

  char addr = 0;

  SDI12Status status = SDI12_OK;
  status = SDI12GetAddress(&addr, 5000);
  if (status != SDI12_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to get address of device: %d\r\n",
            status);
    APP_LOG(TS_OFF, VLEVEL_M,
            "Make sure the device is connected and powered on.\r\n");
    while (1);  // infinite loop at program end
  }

  APP_LOG(TS_OFF, VLEVEL_M, "Current address: %c\r\n", addr);

  HAL_Delay(1000);

  // change the address
  APP_LOG(TS_OFF, VLEVEL_M, "Changing address to 0\r\n");
  char new_addr[] = "xA0!";
  new_addr[0] = addr;
  status = SDI12SendCommand(new_addr, sizeof(new_addr));

  HAL_Delay(1000);

  status = SDI12_OK;
  status = SDI12GetAddress(&addr, 5000);
  if (status != SDI12_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to get address of device: %d\r\n",
            status);
    APP_LOG(TS_OFF, VLEVEL_M,
            "Make sure the device is connected and powered on.\r\n");
    while (1);  // infinite loop at program end
  }
  APP_LOG(TS_OFF, VLEVEL_M, "Current address: %c\r\n", addr);

  while (1);
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
     number, ex printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
