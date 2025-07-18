/**
 * @example example_sdi12.c
 *
 * Reads the measurement from a SDI-12 sensor and prints the raw measurement
 * over serial on a loop. Change the address to match the connected sensor.
 *
 * @author John Madden
 * @date 2023-11-17
 */

// stdlib includes
#include <stdio.h>

// system includes
#include "app_lorawan.h"
#include "board.h"
#include "gpio.h"
#include "lptim.h"
#include "main.h"
#include "rtc.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "sdi12.h"

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
  /* Reset of all peripherals,
  Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();

  SystemApp_Init();

  // User level initialization

  // Print the compilation time at startup
  char info_str[128];
  int info_len;
  info_len = snprintf(
      info_str, sizeof(info_str),
      "Soil Power Sensor Wio-E5 firmware, test: %s, compiled on %s %s\n",
      __FILE__, __DATE__, __TIME__);
  HAL_UART_Transmit(&huart1, (const uint8_t *)info_str, info_len, 1000);
  char success[] = "HAL_OK\n";
  char failure[] = "HAL_FAIL\n";
  char buffer[20];
  uint8_t addr = '0';
  SDI12_Measure_TypeDef measurment_info;

  // Infinite loop
  while (1) {
    // Print voltage level
    char buf[32];
    int buf_len = snprintf(buf, sizeof(buf), "0M!");

    // NOLINTNEXTLINE
    if (SDI12GetMeasurment(addr, &measurment_info, buffer, 3000) == HAL_OK) {
      HAL_UART_Transmit(&huart1, (const uint8_t *)success, 7, 100);
      HAL_UART_Transmit(&huart1, buffer, 18, 100);
    } else {
      HAL_UART_Transmit(&huart1, (const uint8_t *)failure, 10, 100);
    }

    // Sleep
    for (int i = 0; i <= 1000000; i++) {
      asm("nop");
    }
    // HAL_Delay(DELAY);
  }
}

