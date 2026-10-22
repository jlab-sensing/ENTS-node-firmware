#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "ads.h"
#include "app_lorawan.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"
#include "sdi12.h"
#include "solenoid.h"
#include "sys_app.h"
#include "usart.h"
#include "waterPressure.h"

int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  // MX_LPUART1_UART_Init();
  MX_I2C1_Init();
  SystemApp_Init();

  APP_LOG(TS_OFF, VLEVEL_H,
          "example_solenoid.c . Attach DC latching solenoid to pin 18 (A15) in "
          "NO position.\r\n");

  SolenoidInit(18, SOLENOID_TYPE_LATCH_NO);

  while (1) {
    // Open solenoid
    APP_LOG(TS_OFF, VLEVEL_H, "Opening solenoid...\r\n");
    SolenoidChangeState(18, SOLENOID_STATE_ON);

    HAL_Delay(5000);

    // Close solenoid
    APP_LOG(TS_OFF, VLEVEL_H, "Closing solenoid...\r\n");
    SolenoidChangeState(18, SOLENOID_STATE_OFF);

    HAL_Delay(5000);
  }
  /* USER CODE END 3 */
}