/**
 * @example example_rawTeros21.c
 *
 * Uses low level SDI12 functions to get raw measurement strings from the
 * Teros21 sensor and prints the result over UART.

 * @author John Madden
 * @date 2023-11-17
 */

// stdlib includes
#include <stdio.h>

// system includes
#include "app_lorawan.h"
#include "gpio.h"
#include "lptim.h"
#include "main.h"
#include "rtc.h"
#include "sdi12.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "teros21.h"

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
  // NOTE Must be a 18 char buffer
  char buffer[18] = {};
  uint8_t addr = '0';
  SDI12_Measure_TypeDef measurement_info;
  // HAL_StatusTypeDef returnCode;

  // Infinite loop
  while (1) {
    SDI12GetMeasurement(addr, &measurement_info, buffer, 3000);
    APP_PRINTF("test: %s\n", buffer);
    HAL_UART_Transmit(&huart1, buffer, sizeof(buffer), 100);

    /*
    Teros21Data data = {};
    SDI12Status status = SDI12_OK;
    status = Teros21GetMeasurement('0', &data);

    APP_PRINTF("Status code: %d\r\n", status);
    APP_PRINTF("Water potential: %f, Temperature: %f\r\n", data.matric_pot,
    data.temp);
    */

    /*
    if (SDI12GetMeasurement(addr, &measurement_info,  buffer, 3000) == HAL_OK){
      HAL_UART_Transmit(&huart1, (const uint8_t *) success, 7, 100);
      HAL_UART_Transmit(&huart1, buffer, 18, 100);
    } else {
      HAL_UART_Transmit(&huart1, (const uint8_t *) failure, 10, 100);
    };
    */

    // clear buffer
    memset(buffer, 0, sizeof(buffer));

    // Sleep
    for (int i = 0; i <= 1000000; i++) {
      asm("nop");
    }
  }
}
