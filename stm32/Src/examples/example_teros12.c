/**
 * @example example_teros12.c
 *
 * Prints teros12 measurements over serial.
 *
 * @author John Madden
 * @date 2025-02-11
 */

// stdlib includes
#include <stdio.h>
#include <stdlib.h>

// system includes
#include "adc.h"
#include "app_lorawan.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "teros12.h"

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
  APP_LOG(TS_OFF, VLEVEL_M, "Example teros12, compiled on %s %s\r\n", __DATE__,
          __TIME__);

  /* USER CODE BEGIN WHILE */
  while (1) {
    Teros12Data data = {};
    SDI12Status status = SDI12_OK;
    status = Teros12GetMeasurement('0', &data);

    char print_buffer[256];

    snprintf(print_buffer, sizeof(print_buffer),
             "Status code: %d; addr = %c; vwc: %f; temp: %f; ec: %d", status,
             data.addr, data.vwc, data.temp, data.ec);

    APP_PRINTF("%s\r\n", print_buffer);

    // Sleep
    for (int i = 0; i <= 4000000; i++) {
      asm("nop");
    }
  }
}
