/**
 * Example getting the user config from the esp32.
 *
 * The esp32 should have the release code with WiFi enabled.
 *
 * @author Ahmed
 * @date 2025-08-06
 */

#include <stdio.h>
#include <string.h>

#include "board.h"
#include "communication.h"
#include "controller/controller.h"
#include "controller/wifi.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "rtc.h"
#include "stm32_adv_trace.h"
#include "stm32_systime.h"
#include "usart.h"
#include "userConfig.h"
#include "wifi.h"

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
//       APP_LOG(TS_OFF, VLEVEL_M, "Config request failed with status: %d\r\n",
//       status); return;
//   }

//   // 2. Load configuration from FRAM
//   UserConfigStatus fram_status = UserConfigLoad();
//   if (fram_status != USERCONFIG_OK) {
//       if (fram_status == USERCONFIG_FRAM_ERROR) {
//           APP_LOG(TS_OFF, VLEVEL_M, "FRAM access error\r\n");
//       } else if (fram_status == USERCONFIG_DECODE_ERROR) {
//           APP_LOG(TS_OFF, VLEVEL_M, "Config decode error\r\n");
//       } else {
//           APP_LOG(TS_OFF, VLEVEL_M, "Unknown config error: %d\r\n",
//           fram_status);
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
//   APP_LOG(TS_OFF, VLEVEL_M, "Upload Intvl: %lu sec\r\n",
//   config->Upload_interval); APP_LOG(TS_OFF, VLEVEL_M, "WiFi SSID:    %s\r\n",
//   config->WiFi_SSID); APP_LOG(TS_OFF, VLEVEL_M, "API Endpoint: %s:%lu\r\n",
//          config->API_Endpoint_URL, config->API_Endpoint_Port);
//   APP_LOG(TS_OFF, VLEVEL_M, "----------------------\r\n");

//   // 3. Send configuration to ESP32
//   UserConfigCommand response = {0};
//   response.type = UserConfigCommand_RequestType_RESPONSE_CONFIG;
//   response.has_config_data = true;
//   memcpy(&response.config_data, config, sizeof(UserConfiguration));

//   tx->len = EncodeUserConfigCommand(response.type, &response.config_data,
//   tx->data, tx->size); if (tx->len == 0) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Failed to encode config response\r\n");
//       return;
//   }

//   status = ControllerTransaction(g_controller_i2c_timeout);
//   if (status == CONTROLLER_SUCCESS) {
//       APP_LOG(TS_OFF, VLEVEL_M, "Configuration successfully sent to
//       ESP32\r\n");
//   } else {
//       APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config (status: %d)\r\n",
//       status);
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
      APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config to ESP32: %d\r\n",
              status);
    }
  }
}

int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  SystemApp_Init();

  ControllerInit();
  GetCurrentConfigFromSTM32();
  while (1) {}
}
