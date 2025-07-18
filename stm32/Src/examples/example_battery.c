/**
 * @example example_battery.c
 *
 * In an infinite loop the battery voltage level is retrieved then outputted
 * over serial. The user should check if the voltage levels are expected. When
 * connected to USB the voltage should be ~5V. The battery voltage level should
 * be checked and compared to a multimeter measurement.
 *
 * @author John Madden
 * @date 2023-11-17
 */

#include <stdio.h>

#include "adc.h"
#include "battery.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "main.h"
#include "usart.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

/** Global variable for all return codes */
HAL_StatusTypeDef rc;

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
  MX_ADC_Init();
  MX_USART1_UART_Init();

  // User level initialization
  battery_init();

  // Print the compilation time at startup
  char info_str[128];
  int info_len;
  info_len = snprintf(
      info_str, sizeof(info_str),
      "Soil Power Sensor Wio-E5 firmware, test: %s, compiled on %s %s\n",
      __FILE__, __DATE__, __TIME__);
  HAL_UART_Transmit(&huart1, (const uint8_t *)info_str, info_len, 1000);

  // Infinite loop
  while (1) {
    // Print voltage level
    char buf[32];
    int buf_len = snprintf(buf, sizeof(buf), "Battery Voltage: %d mV\n",
                           battery_voltage());
    HAL_UART_Transmit(&huart1, (const uint8_t *)buf, buf_len, 1000);

    // Sleep
    // HAL_Delay(DELAY);

    for (int i = 0; i < 1000000; i++) {
      __NOP();
    }
  }
}

