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

  MX_ADC_Init();  // also called within SystemApp_Init()

  // board revision 3.1.0 ADC quick reference
  // Channel | Port & Pin | 40-pin | Note / Primary Function
  //  0        PB13         16   User (boot) button & TCA9535 IO expander int
  //  1        PB14         21   SPI2 MISO (no onboard SPI devices, ok to use)
  //  2        PB3          22
  //  3        PB4          24
  //  6        PA10          3   I2C1 SDA (NOT RECOMMENDED FOR ADC)
  // 11        PA15         18

  // ADC_ChannelConfTypeDef sConfig = {0};
  // sConfig.Channel = ADC_CHANNEL_1;
  // rank 1 and 2 used for ADC_CHANNEL_TEMPSENSOR and ADC_CHANNEL_VREFINT
  // sConfig.Rank = ADC_REGULAR_RANK_3;
  // sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
  // if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
  //   Error_Handler();
  // }
  // sConfig.Channel = ADC_CHANNEL_2;
  // sConfig.Rank = ADC_REGULAR_RANK_4;
  // if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
  //   Error_Handler();
  // }
  // sConfig.Channel = ADC_CHANNEL_3;
  // sConfig.Rank = ADC_REGULAR_RANK_5;
  // if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
  //   Error_Handler();
  // }
  // sConfig.Channel = ADC_CHANNEL_11;
  // sConfig.Rank = ADC_REGULAR_RANK_6;
  // if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK) {
  //   Error_Handler();
  // }

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  GPIO_InitStruct.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_14;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  // init systemapp
  SystemApp_Init();

  APP_PRINTF("example_watermark200SSVA3, compiled on %s %s\r\n", __DATE__,
             __TIME__);

  // HAL_StatusTypeDef error = HAL_OK;
  uint32_t value_raw = 0;
  double value_voltage = 0.0;
  double tension_kPa = 0.0;
  double temperature_f = 0.0;
  double temperature_c = 0.0;
  uint32_t conversion_count = 0;

  // HAL_ADC_GetState(&hadc);
  while (1) {
    // error = HAL_ADC_Start(&hadc);
    // if (error != HAL_OK) APP_PRINTF("HAL_ADC_Start error %d\r\n", error);

    // error = HAL_ADC_PollForConversion(&hadc, 1000);
    // if (error != HAL_OK) APP_PRINTF("HAL_ADC_Start error %d\r\n", error);

    // value = HAL_ADC_GetValue(&hadc);

    // error = HAL_ADC_Stop(&hadc);
    // if (error != HAL_OK) APP_PRINTF("HAL_ADC_Start error %d\r\n", error);

    value_raw = ADC_Convert_Single(ADC_CHANNEL_1);
    value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
    tension_kPa = value_voltage / 0.0117;
    APP_PRINTF("%d Soil Tension (WM1): %lf kPa (%lf V ; %u raw)\r\n",
               conversion_count, tension_kPa, value_voltage, value_raw);
    value_raw = ADC_Convert_Single(ADC_CHANNEL_2);
    value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
    tension_kPa = value_voltage / 0.0117;
    APP_PRINTF("%d Soil Tension (WM2): %lf kPa (%lf V ; %u raw)\r\n",
               conversion_count, tension_kPa, value_voltage, value_raw);
    value_raw = ADC_Convert_Single(ADC_CHANNEL_3);
    value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
    tension_kPa = value_voltage / 0.0117;
    APP_PRINTF("%d Soil Tension (WM3): %lf kPa (%lf V ; %u raw)\r\n",
               conversion_count, tension_kPa, value_voltage, value_raw);
    value_raw = ADC_Convert_Single(ADC_CHANNEL_11);
    value_voltage = (double)value_raw * 3.3 / ((1 << 12) - 1);
    temperature_f = 50.68 * (value_voltage - 0.490) + 20;
    temperature_c = (temperature_f - 32) * 5.0 / 9.0;
    APP_PRINTF("%d Soil Temperature:   %lf F %lf C   (%lf V ; %u raw)\r\n",
               conversion_count, temperature_f, temperature_c, value_voltage, value_raw);

    APP_PRINTF("\r\n");

    conversion_count++;

    HAL_Delay(2000);
  }
}
