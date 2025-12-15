#include "board.h"

// system includes
#include "stm32wlxx_hal.h"

// peripherials
#include "adc.h"
#include "app_lorawan.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"
#include "sys_app.h"
#include "tim.h"
#include "usart.h"

// user includes
#include "status_led.h"

void Board_Init(void) {
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();  
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  
  // required for SDI-12
  MX_USART2_UART_Init();
  MX_TIM1_Init();
}

void Board_DeInit(void) {
  if (HAL_ADC_DeInit(&hadc) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_UART_DeInit(&huart1) != HAL_OK) {
    Error_Handler();
  }

  // NOTE DMA does not have a deinit function, STOP2 clocks are sufficient

  if (HAL_I2C_DeInit(&hi2c2) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_UART_DeInit(&huart2) != HAL_OK) {
    Error_Handler();
  }

  if (HAL_TIM_Base_DeInit(&htim1) != HAL_OK) {
    Error_Handler();
  }
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  // char error[30];
  // int error_len = sprintf(error, "Error!  HAL Status: %d\n", rc);
  // HAL_UART_Transmit(&huart1, (const uint8_t *)error, error_len, 1000);

  StatusLedOn();

  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

void WaitForSerial(void)
{
  // Wait 2 seconds for serial to be ready
  int seconds = 2;
  int cycles = seconds * MSI_VALUE;
  for (int i = 0; i < cycles; i++)
  {
    __NOP();
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
   * example: printf("Wrong parameters value: file %s on line %d\r\n", file, line)
   */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
