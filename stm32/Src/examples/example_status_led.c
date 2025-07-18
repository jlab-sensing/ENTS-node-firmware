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
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  SystemApp_Init();

  StatusLedInit();

  APP_LOG(TS_OFF, VLEVEL_M, "Default status. Expected off.\n");
  PrintLedState();
  HAL_Delay(5000);

  StatusLedOn();
  APP_LOG(TS_OFF, VLEVEL_M, "Turning on\n");
  PrintLedState();
  HAL_Delay(5000);

  StatusLedOff();
  APP_LOG(TS_OFF, VLEVEL_M, "Turning off\n");
  PrintLedState();
  HAL_Delay(5000);

  APP_LOG(TS_OFF, VLEVEL_M, "Done\n");

  while (1) {
  }
}

