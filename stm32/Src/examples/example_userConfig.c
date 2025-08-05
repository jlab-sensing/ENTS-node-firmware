/**
 * @file example_userConfig.c
 * @brief Prints the current user config
 */

// system includes
#include "board.h"
#include "gpio.h"
#include "main.h"
#include "rtc.h"

// user includes
#include "userConfig.h"

char uart_buf[512];

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
