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
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "rtc.h"
#include "userConfig.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
char uart_buf[512];
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
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  /*Initialize timer and RTC*/
  /*Have to be initilized in example files because LoRaWan cannot be initialized
   * like in main*/
  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  UTIL_TIMER_Init();
  /* USER CODE BEGIN 2 */
  // UserConfig_InterruptInit();  // Initialize UART for interrupt mode

  // Load user configuration from FRAM
  if (UserConfigLoad() == USERCONFIG_OK) {
    // Get a pointer to the loaded configuration
    const UserConfiguration *config = UserConfigGet();

    // Print each member of the UserConfiguration
    sprintf(uart_buf, "Logger ID: %lu\r\n", config->logger_id);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "Cell ID: %lu\r\n", config->cell_id);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    if (config->Upload_method == 0) {
      sprintf(uart_buf, "Upload Method: %u \"LoRa\"\r\n",
              config->Upload_method);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                        HAL_MAX_DELAY);
    } else {
      sprintf(uart_buf, "Upload Method: %u \"WiFi\"\r\n",
              config->Upload_method);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                        HAL_MAX_DELAY);
    }

    sprintf(uart_buf, "Upload Interval: %lu\r\n", config->Upload_interval);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    for (int i = 0; i < config->enabled_sensors_count; i++) {
      const char *sensor_name;
      switch (config->enabled_sensors[i]) {
        case 0:
          sensor_name = "Voltage";
          break;
        case 1:
          sensor_name = "Current";
          break;
        case 2:
          sensor_name = "Teros12";
          break;
        case 3:
          sensor_name = "Teros21";
          break;
        case 4:
          sensor_name = "BME280";
          break;
      }
      sprintf(uart_buf, "Enabled Sensor %d: %s\r\n", i + 1, sensor_name);
      HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                        HAL_MAX_DELAY);
    }

    sprintf(uart_buf, "Calibration V Slope: %f\r\n", config->Voltage_Slope);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "Calibration V Offset: %f\r\n", config->Voltage_Offset);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "Calibration I Slope: %f\r\n", config->Current_Slope);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "Calibration I Offset: %f\r\n", config->Current_Offset);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "WiFi SSID: %s\r\n", config->WiFi_SSID);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "WiFi Password: %s\r\n", config->WiFi_Password);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "API Endpoint URL: %s\r\n", config->API_Endpoint_URL);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

    sprintf(uart_buf, "API Port: %lu\r\n", config->API_Endpoint_Port);
    HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, strlen(uart_buf),
                      HAL_MAX_DELAY);

  } else {
    const char *error_msg = "Failed to load user configuration from FRAM.\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t *)error_msg, strlen(error_msg),
                      HAL_MAX_DELAY);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_11;
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
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
