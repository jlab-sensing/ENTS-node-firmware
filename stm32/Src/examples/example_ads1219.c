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
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  // init systemapp
  SystemApp_Init();

  APP_PRINTF("example_adc, compiled on %s %s\r\n", __DATE__, __TIME__);

  // Calibration values. Leave as 1 0 1 0 to read raw values.
  double v_m = 1, v_b = 0, i_m = 1, i_b = 0;

  // A1   A0   I2C ADDRESS
  // DGND DGND 100 0000 (0x40)
  // DGND DVDD 100 0001
  // DGND SDA  100 0010
  // DGND SCL  100 0011
  // DVDD DGND 100 0100
  // DVDD DVDD 100 0101
  // DVDD SDA  100 0110
  // DVDD SCL  100 0111
  // SDA  DGND 100 1000
  // SDA  DVDD 100 1001
  // SDA  SDA  100 1010
  // SDA  SCL  100 1011
  // SCL  DGND 100 1100
  // SCL  DVDD 100 1101
  // SCL  SDA  100 1110
  // SCL  SCL  100 1111

  UserConfiguration cfg = UserConfiguration_init_zero;
  cfg.enabled_sensors_multiple_count = 2;  // not used in this example

  cfg.enabled_sensors_multiple[0].cell_id = 200;  // not used in this example
  cfg.enabled_sensors_multiple[0].enabled_sensor = EnabledSensor_Current;
  cfg.enabled_sensors_multiple[0].index = 0x40 << 1;
  cfg.enabled_sensors_multiple[1].cell_id = 200;  // not used in this example
  cfg.enabled_sensors_multiple[1].enabled_sensor = EnabledSensor_Voltage;
  cfg.enabled_sensors_multiple[1].index = 0x40 << 1;

  cfg.enabled_sensors_multiple[2].cell_id = 201;  // not used in this example
  cfg.enabled_sensors_multiple[2].enabled_sensor = EnabledSensor_Current;
  cfg.enabled_sensors_multiple[2].index = 0x41 << 1;
  cfg.enabled_sensors_multiple[3].cell_id = 201;  // not used in this example
  cfg.enabled_sensors_multiple[3].enabled_sensor = EnabledSensor_Voltage;
  cfg.enabled_sensors_multiple[3].index = 0x41 << 1;

  // Technically, ADC_init only needs to be called once per device.
  ADC_init(&(cfg.enabled_sensors_multiple[0]), v_m, v_b, i_m, i_b);
  ADC_init(&(cfg.enabled_sensors_multiple[1]), v_m, v_b, i_m, i_b);

  // When initializing another ADS1219, make sure to use the appropriate
  // calibration values.
  // TODO: Fetch the calibration values stored within the ADS1219 board's
  // onboard FRAM.
  ADC_init(&(cfg.enabled_sensors_multiple[2]), v_m, v_b, i_m, i_b);
  ADC_init(&(cfg.enabled_sensors_multiple[3]), v_m, v_b, i_m, i_b);

  double voltage_reading;
  double current_reading;

  while (1) {
    // device 1
    voltage_reading = ADC_readVoltage(&(cfg.enabled_sensors_multiple[0]));
    APP_PRINTF("Voltage (%02X): %lf\r\n", cfg.enabled_sensors_multiple[0].index,
               voltage_reading);

    current_reading = ADC_readCurrent(&(cfg.enabled_sensors_multiple[1]));
    APP_PRINTF("Current (%02X): %lf\r\n", cfg.enabled_sensors_multiple[1].index,
               current_reading);

    // device 2
    voltage_reading = ADC_readVoltage(&(cfg.enabled_sensors_multiple[2]));
    APP_PRINTF("Voltage (%02X): %lf\r\n", cfg.enabled_sensors_multiple[2].index,
               voltage_reading);

    current_reading = ADC_readCurrent(&(cfg.enabled_sensors_multiple[3]));
    APP_PRINTF("Current (%02X): %lf\r\n", cfg.enabled_sensors_multiple[3].index,
               current_reading);

    HAL_Delay(5000);
  }
  /* USER CODE END 3 */
}
