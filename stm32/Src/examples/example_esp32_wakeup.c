/**
 * @example example_esp32_wakeup.c
 *
 * Triggers the esp32 wakeup pin on an interval to wake up the esp32 from
 * a deep sleep state. Load the environment `example_deepsleep` on the esp32
 * for testing.
 *
 * @author John Madde <jmadden173@pm.me>
 * @date 2025-11-04
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

const unsigned int wakeup_interval_ms = 10000;

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

  // ensure wakeup pin is low
  HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin, GPIO_PIN_RESET);

  // hard initialize gpio pin
  // override any global gpio configuration
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = ESP32_WAKEUP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(ESP32_WAKEUP_GPIO_Port, &GPIO_InitStruct);

  while (1) {

    // set wakeup pin
    HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin,
                          GPIO_PIN_SET);
    StatusLedOn();

    HAL_Delay(100);

    // reset wakeup pin
    HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin,
                          GPIO_PIN_RESET);
    StatusLedOff(); 

    APP_LOG(TS_ON, VLEVEL_M, "Triggered wakeup pin\n");

    HAL_Delay(wakeup_interval_ms);
  }
}
