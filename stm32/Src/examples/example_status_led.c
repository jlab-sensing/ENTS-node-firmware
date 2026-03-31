/**
 * @example  example_status_led.c
 *
 * Example code for using the status LED on the Wio-E5 board. Changes through
 * all statuses of the LED and prints the state to the console.
 *
 * @author John Madde <jmadden173@pm.me>
 * @date 2025-04-17
 */

// stdlib includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// system includes
#include "adc.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "status_led.h"
#include "sys_app.h"
#include "tim.h"
#include "usart.h"

void PrintLedState(void) {
  int state = HAL_GPIO_ReadPin(USER_LED_GPIO_Port, USER_LED_Pin);
  APP_LOG(TS_OFF, VLEVEL_M, "LED Pin State: %d\n", state);
}

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  // MX_ADC_Init();
  MX_USART2_UART_Init();
  // MX_I2C1_Init();
  SystemApp_Init();

  char msg_init[] = "Initializing status LED.\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t *)msg_init, sizeof(msg_init), HAL_MAX_DELAY);

  StatusLedInit();

  APP_LOG(TS_OFF, VLEVEL_M, "Default status. Expected off.\n");
  PrintLedState();
  HAL_Delay(1000);

  char msg_enter_loop[] = "Entering loop.\r\n";
  HAL_UART_Transmit(&huart2, (uint8_t *)msg_enter_loop, sizeof(msg_enter_loop), HAL_MAX_DELAY);

  uint32_t i = 0;
  char msg[] = "000\r\n";

  while (1) {
    msg[2] = '0' + (i % 10);
    msg[1] = '0' + ((i / 10) % 10);
    msg[0] = '0' + ((i / 100) % 10);

    HAL_UART_Transmit(&huart2, (uint8_t *)msg, sizeof(msg), HAL_MAX_DELAY);

    StatusLedOn();
    APP_LOG(TS_OFF, VLEVEL_M, "Turning on\n");
    PrintLedState();
    HAL_Delay(1000);

    StatusLedOff();
    APP_LOG(TS_OFF, VLEVEL_M, "Turning off\n");
    PrintLedState();
    HAL_Delay(1000);

    i++;
  }
}
