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
#include "controller/controller.h"
#include "controller/wifi_userconfig.h"
#include "gpio.h"
#include "i2c.h"
#include "usart.h"
#include "userConfig.h"

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
  } else {
    UserConfigPrint();
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
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  SystemApp_Init();

  ControllerInit();

  // variables to store WiFi host info
  char ssid[255] = "ents-unconfigured";
  char pass[255] = "ilovedirt";

  ControllerWiFiHost(ssid, pass);
  ControllerUserConfigStart();
  GetCurrentConfigFromSTM32();

  while (1) {}
}
