/**
 * @example example_watermark200SSVA3.c
 *
 * Example to read 4 voltage outputs from a Watermark 200SS VA3 adapter
 * (using the internal ADC) and print out the values.
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
#include "watermark.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  MX_ADC_Init();  // also called within SystemApp_Init(), and by
                  // Watermark200SSVA3_Init()

  // board revision 3.1.0 ADC quick reference
  // Channel | Port & Pin | 40-pin | Note / Primary Function
  //  0        PB13         16   User (boot) button & TCA9535 IO expander int
  //  1        PB14         21   SPI2 MISO (no onboard SPI devices, ok to use)
  //  2        PB3          22
  //  3        PB4          24
  //  6        PA10          3   I2C1 SDA (NOT RECOMMENDED FOR ADC)
  // 11        PA15         18

  Watermark200SSVA3_Init();

  // init systemapp
  SystemApp_Init();

  APP_PRINTF("example_watermark200SSVA3, compiled on %s %s\r\n", __DATE__,
             __TIME__);

  uint32_t conversion_count = 0;

  while (1) {
    Watermark200SSVA3_GetMeasurement();
    APP_PRINTF("%d Soil Tension (WM1): %lf kPa\r\n", conversion_count, Watermark200SSVA3_GetWM1());
    APP_PRINTF("%d Soil Tension (WM2): %lf kPa\r\n", conversion_count, Watermark200SSVA3_GetWM2());
    APP_PRINTF("%d Soil Tension (WM3): %lf kPa\r\n", conversion_count, Watermark200SSVA3_GetWM3());
    APP_PRINTF("%d Soil Temperature:   %lf C\r\n", conversion_count, Watermark200SSVA3_GetWMTemp());

    APP_PRINTF("\r\n");

    conversion_count++;

    HAL_Delay(2000);
  }
}
