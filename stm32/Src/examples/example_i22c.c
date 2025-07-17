#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "controller/wifi.h"
#include "controller/controller.h"
#include "userconfig.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "stm32_adv_trace.h"
#include <string.h>
#include <stdio.h>
#include "wifi.h"
#include "communication.h"
#include "rtc.h"
#include "stm32_systime.h"

char uart_buf[512];

// /** Timeout for i2c communication with esp32 */
// static unsigned int g_controller_i2c_timeout = 10000;

// void GetCurrentConfigFromSTM32() {
//   Buffer *tx = ControllerTx();
//   Buffer *rx = ControllerRx();

//   memset(tx->data, 0, tx->size);
//   memset(rx->data, 0, rx->size);
//   tx->len = 0;
//   rx->len = 0;

//   // 1. Send initial request to ESP32
//   UserConfigCommand request = {0};
//   request.type = UserConfigCommand_RequestType_REQUEST_CONFIG;
//   request.has_config_data = false;

//   tx->len = EncodeUserConfigCommand(request.type, NULL, tx->data, tx->size);
//   if (tx->len == 0) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config request\r\n");
//       return;
//   }

//   ControllerStatus status = ControllerTransaction(g_controller_i2c_timeout);
//   if (status != CONTROLLER_SUCCESS) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Config request failed with status: %d\r\n", status);
//       return;
//   }

//   // 2. Load configuration from FRAM
//   UserConfigStatus fram_status = UserConfigLoad();
//   if (fram_status != USERCONFIG_OK) {
//       if (fram_status == USERCONFIG_FRAM_ERROR) {
//           APP_LOG(TS_OFF, VLEVEL_M, "FRAM access error\r\n");
//       } else if (fram_status == USERCONFIG_DECODE_ERROR) {
//           APP_LOG(TS_OFF, VLEVEL_M, "Config decode error\r\n");
//       } else {
//           APP_LOG(TS_OFF, VLEVEL_M, "Unknown config error: %d\r\n", fram_status);
//       }
//       return;
//   }

//   const UserConfiguration *config = UserConfigGet();
//   if (config == NULL) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Null config received from FRAM\r\n");
//       return;
//   }

//   APP_LOG(TS_OFF, VLEVEL_M, "Current Configuration:\r\n");
//   APP_LOG(TS_OFF, VLEVEL_M, "----------------------\r\n");
//   APP_LOG(TS_OFF, VLEVEL_M, "Logger ID:    %lu\r\n", config->logger_id);
//   APP_LOG(TS_OFF, VLEVEL_M, "Cell ID:      %lu\r\n", config->cell_id);
//   APP_LOG(TS_OFF, VLEVEL_M, "Upload Method: %s\r\n", 
//          config->Upload_method == Uploadmethod_LoRa ? "LoRa" : "WiFi");
//   APP_LOG(TS_OFF, VLEVEL_M, "Upload Intvl: %lu sec\r\n", config->Upload_interval);
//   APP_LOG(TS_OFF, VLEVEL_M, "WiFi SSID:    %s\r\n", config->WiFi_SSID);
//   APP_LOG(TS_OFF, VLEVEL_M, "API Endpoint: %s:%lu\r\n", 
//          config->API_Endpoint_URL, config->API_Endpoint_Port);
//   APP_LOG(TS_OFF, VLEVEL_M, "----------------------\r\n");

//   // 3. Send configuration to ESP32
//   UserConfigCommand response = {0};
//   response.type = UserConfigCommand_RequestType_RESPONSE_CONFIG;
//   response.has_config_data = true;
//   memcpy(&response.config_data, config, sizeof(UserConfiguration));

//   tx->len = EncodeUserConfigCommand(response.type, &response.config_data, tx->data, tx->size);
//   if (tx->len == 0) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config response\r\n");
//       return;
//   }

//   status = ControllerTransaction(g_controller_i2c_timeout);
//   if (status == CONTROLLER_SUCCESS) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Configuration successfully sent to ESP32\r\n");
//   } else {
//       APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config (status: %d)\r\n", status);
//   }
// }


void GetCurrentConfigFromSTM32() {
  // 1. First try to get config from ESP32
  APP_LOG(TS_OFF, VLEVEL_M, "Requesting configuration from ESP32...\r\n");
  UserConfigStatus status = ControllerUserConfigRequest();
  
  if (status == USERCONFIG_EMPTY_CONFIG || status != USERCONFIG_OK) {
      // 2. If ESP32 has empty config or request failed, send our config
      APP_LOG(TS_OFF, VLEVEL_M, "Sending FRAM configuration to ESP32...\r\n");
      status = ControllerUserConfigSend();
      
      if (status != USERCONFIG_OK) {
          APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config to ESP32: %d\r\n", status);
      }
  }
}

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
    MX_ADC_Init();
    MX_USART1_UART_Init();
    MX_I2C2_Init(); 
    SystemApp_Init();
  
    ControllerInit();
    GetCurrentConfigFromSTM32();
  while (1) {
    
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
  