#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "adc.h"
#include "ads.h"
#include "app_lorawan.h"
#include "board.h"
#include "dma.h"
#include "esp_link.h"
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
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();

  __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  UTIL_TIMER_Init();
  UserConfigLoad();

  char info_str[100];
  int info_len;
  info_len = snprintf(info_str, sizeof(info_str),
                      "Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n",
                      __DATE__, __TIME__);
  HAL_UART_Transmit(&huart1, (const uint8_t *)info_str, info_len, 1000);

  SolenoidInit();

  char output[30];

  size_t reading_len;

  while (1) {
    SolenoidOpen();

    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
      reading_len = snprintf(output, sizeof(output), "Solenoid Open\r\n");
      HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                        HAL_MAX_DELAY);
    } else {
      reading_len =
          snprintf(output, sizeof(output), "Solenoid is NOT Open\r\n");
      HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                        HAL_MAX_DELAY);
    }

    HAL_Delay(5000);  // 5000 ms = 5 seconds

    // Close solenoid
    SolenoidClose();

    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_SET) {
      reading_len = snprintf(output, sizeof(output), "Solenoid Closed\r\n");
      HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                        HAL_MAX_DELAY);
    } else {
      reading_len =
          snprintf(output, sizeof(output), "Solenoid is NOT Closed\r\n");
      HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                        HAL_MAX_DELAY);
    }

    HAL_Delay(5000);  // 5000 ms = 5 seconds
  }
  /* USER CODE END 3 */
}
