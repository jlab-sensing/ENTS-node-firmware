/**
 * @example example_adc.c
 *
 * Example continuously reading voltage and current from the ADC.
 */

// stdlib includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// system includes
#include "adc.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "ads.h"
#include "sdi12.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  // init systemapp
  SystemApp_Init();

  UserConfigLoad();

  APP_PRINTF("example_adc, compiled on %s %s\r\n", __DATE__, __TIME__);

  /* USER CODE BEGIN 2 */
  // HAL_Delay(50);
  ADC_init();
  // int size = sprintf(fail_str, "Failed at ADC_init\n");
  // HAL_UART_Transmit(&huart1, (const uint8_t *) fail_str, size, 100);

  double voltage_reading;
  double current_reading;
  size_t reading_len;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // Looked into enabling floating prints with printf and scanf but did not
    // work. Fix is tbd

    voltage_reading = ADC_readVoltage();
    char voltage_str[20];
    snprintf(voltage_str, sizeof(voltage_str), "%lf", voltage_reading);
    APP_PRINTF("Voltage: %s\r\n", voltage_str);

    current_reading = ADC_readCurrent();
    char current_str[20];
    snprintf(current_str, sizeof(current_str), "%f", current_reading);
    APP_PRINTF("Current: %s\r\n", current_str);

    HAL_Delay(1000);
  }
  /* USER CODE END 3 */
}
