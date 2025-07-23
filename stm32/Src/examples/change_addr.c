/**
 * @example change_addr.c
 *
 * Change the address of a SDI-12 sensor. Instructions are as follows:
 *
 * 1. Adjust the target address from 0 to a non-zero target address in
 *    change_addr.c:65
 *   - The factory-default address for the TEROS-12 is 0, so the target address
 *   should be changed in order to observe a change in address (0 -->
 *   non-zero).
 *    - `char new_addr[] = "xA0!";` <-- Change the character `0` to a non-zero
 *    number (or letter)
 * 2. Run the example (`pio run -e change_addr`) to change the sensor's
 *    address.
 *
 * @author John Madden
 * @date 2025-02-11
 */

#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "board.h"
#include "app_lorawan.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "sys_app.h"
#include "teros12.h"
#include "usart.h"

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

