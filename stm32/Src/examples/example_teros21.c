/**
 * @example example_teros12.c
 *
 * Prints teros21 measurements over serial.
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
#include "sdi12.h"
#include "teros21.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();

  SystemApp_Init();
  /* USER CODE BEGIN 2 */

  // Print the compilation time at startup
  APP_LOG(TS_OFF, VLEVEL_M, "Example teros21, compiled on %s %s\r\n", __DATE__,
          __TIME__);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    Teros21Data data = {};
    SDI12Status status = SDI12_OK;
    status = Teros21GetMeasurement('0', &data);

    APP_PRINTF("Status code: %d\r\n", status);

    char print_buffer[256];
    snprintf(print_buffer, sizeof(print_buffer),
             "Water potential: %f, Temperature: %f\r\n", data.matric_pot,
             data.temp);
    APP_PRINTF("%s", print_buffer);

    // Sleep
    for (int i = 0; i <= 4000000; i++) {
      asm("nop");
    }
    // HAL_Delay(DELAY);
  }
}
