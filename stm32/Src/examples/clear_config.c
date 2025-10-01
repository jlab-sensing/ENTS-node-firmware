/**
 * @example clear_config.c
 *
 * Clears the user configuration stored in FRAM.
 *
 * @author John Madden
 * @date 2025-10-01
 */

// stdlib includes
#include <stdio.h>

// system includes
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"
#include "userConfig.h"

/**
 * @brief Entry point for battery test
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();

  // init system app
  SystemApp_Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Clear config (%s), compiled on %s %s\r\n",
          __FILE__, __DATE__, __TIME__);

  UserConfigStatus status = UserConfigLoad();
  if (status != USERCONFIG_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "No valid configuration found: %d\n", status);
    while (1) {
      asm("nop");
    }
  }

  const UserConfiguration *config = UserConfigGet();

  APP_LOG(TS_OFF, VLEVEL_M, "Current configuration:\n");
  UserConfigPrintAny(config);

  status = UserConfigClear();
  if (status != USERCONFIG_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to clear configuration: %d\n", status);
    APP_LOG(TS_OFF, VLEVEL_M, "Retrying...\n");
  }
}
