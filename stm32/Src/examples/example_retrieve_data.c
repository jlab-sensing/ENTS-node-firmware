#include <stdio.h>

#include "adc.h"
#include "battery.h"
#include "dma.h"
#include "fifo.h"
#include "fram.h"
#include "gpio.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

void SystemClock_Config(void);
HAL_StatusTypeDef rc;

/**
 * @brief Entry point for managing and retrieving data stored in FRAM across
restarts.

* This code performs the following functions:
* 1. Initializes the STM32 system, GPIO, I2C, UART, and other
application-specific modules.
* 2. Writes example data multiple times to a FRAM buffer to test data storage
functionality.
* 3. Blinks an LED on GPIO_PIN_5 of port GPIOB to indicate the program is
running.
* 4. (Optional) Includes a section to read and print stored data from FRAM,
currently commented out.
*
* Expected behavior: The LED should blink continuously, and pressing the
"Restart" button will
* trigger additional data entries in the FRAM buffer.
*
* @retval int
*/

int main(void) {
  HAL_Init();
  SystemClock_Config();
  SystemApp_Init();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_DMA_Init();
  MX_I2C2_Init();

  FIFO_Init();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Set as push-pull output
  GPIO_InitStruct.Pull = GPIO_NOPULL;          // No pull-up or pull-down
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // Low frequency for LED

  FramStatus status;

  const uint8_t test_data[] = {0x11, 0x22, 0x33};  // Example data
  for (int i = 0; i < 10; i++) {
    status = FramPut(test_data, sizeof(test_data));
  }

  uint16_t read_addr, write_addr, buffer_len;
  status = FramLoadBufferState(&read_addr, &write_addr, &buffer_len);

  uint8_t retrieved_data[sizeof(test_data)];
  uint8_t retrieved_len;

  APP_PRINTF("Press the Restart button to add more data to FRAM...\n");

  // Visualizing actual data (commented out)
  // while (FramBufferLen() > 0) {
  //     status = FramGet(retrieved_data, &retrieved_len);
  //     if (status == FRAM_OK) {
  //         // Loop through retrieved_data and print each byte in hexadecimal
  //         format print("Data length: %d\n", retrieved_len); for (int i = 0; i
  //         < retrieved_len; i++) {
  //             APP_PRINTF("Data[%d]: 0x%02X\n", i, retrieved_data[i]);
  //         }
  //     } else {
  //         // Handle error if needed
  //         break; // Exit the loop if an error occurs
  //     }
  // }

  while (1) {
    char buf[32];
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    HAL_Delay(500);
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
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
