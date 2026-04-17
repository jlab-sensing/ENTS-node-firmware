/**
 * @example example_tca9535.c
 *
 * 
 *
 * @author Jack Lin <jlin143@ucsc.edu>
 * @date 2026-04-17
 */

// stdlib includes
// #include <stdbool.h>
#include <stdio.h>
// #include <stdlib.h>

// system includes
#include "controller/controller.h"
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
#include "tca9535.h"

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  SystemApp_Init();

  StatusLedInit();

  APP_LOG(TS_OFF, VLEVEL_M, "example_tca9535.c\r\n");

  // boot ESP32
  ControllerDeviceEnable();

  if (!TCA9535Init(false)){
    APP_LOG(TS_OFF, VLEVEL_M, "Failed to initialize TCA9535\r\n");
  }

  Tca9535WritePin(TCA9535_P10_PORT, TCA9535_P10_PIN, GPIO_PIN_RESET);
  Tca9535SetDirection(TCA9535_P10_PORT, TCA9535_P10_PIN, TCA9535_CONFIG_INPUT);

  APP_LOG(TS_OFF, VLEVEL_M, "P10 high 1 sec, low 1 sec.\r\n");

  while (1) {
    Tca9535WritePin(TCA9535_P10_PORT, TCA9535_P10_PIN, GPIO_PIN_SET);
    StatusLedOn();

    HAL_Delay(1000);

    Tca9535WritePin(TCA9535_P10_PORT, TCA9535_P10_PIN, GPIO_PIN_RESET);
    StatusLedOff();

    HAL_Delay(1000);
  }
}
