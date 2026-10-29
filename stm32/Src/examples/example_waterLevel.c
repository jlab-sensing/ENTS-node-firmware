/**
 * @example example_waterLevel.c
 *
 * Minimal example for reading a ALS-MPM-2F (TL-136 / GL-136) water level
 * sensor. This water level sensor is (typically) a 4-20 mA output. The STM32
 * reads the output by putting a 150 Ohm resistor in series with the current
 * loop and reading the voltage across the resistor.
 *
 * Wiring Diagram
 *
 *   +-->|+ Sensor -|<--+
 *   |                  |
 *   |                  +-----> p21 ADC1
 *   |                  |
 * 12VDC              150R
 *   |                  |
 *   |                  +-----> p20 GND
 *   |                  |
 *  GND                GND
 *
 * @author Eric Tran & Jack Lin
 * @date 2026-08-19
 */
#include <stdio.h>

#include "adc.h"
#include "dma.h"
#include "gpio.h"
#include "sys_app.h"
#include "usart.h"
#include "userConfig.h"
#include "waterLevel.h"

void SystemClock_Config(void);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  // MX_LPUART1_UART_Init();
  // MX_I2C1_Init();

  SystemApp_Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS,
          "Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n", __DATE__,
          __TIME__);

  UserConfiguration cfg = UserConfiguration_init_zero;
  cfg.enabled_sensors_multiple[0].cell_id = 200;  // not used in this example
  cfg.enabled_sensors_multiple[0].enabled_sensor = EnabledSensor_ALSMPM2F;
  cfg.enabled_sensors_multiple[0].index = 21;  // ADC channel 1, pin 21
  cfg.enabled_sensors_multiple_count = 1;      // not used in this example
  WaterLevelInit(&(cfg.enabled_sensors_multiple[0]));

  ALSMPM2FMeasurement measurement;

  while (1) {
    measurement = WatLevelGetMeasurement(&(cfg.enabled_sensors_multiple[0]));
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Depth: %lf m [%lf V]\r\n",
            measurement.meters, measurement.voltage);

    HAL_Delay(1000);
  }
}
