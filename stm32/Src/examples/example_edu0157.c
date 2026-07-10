/**
 * @file example_edu0157.c
 *
 * Prints the raw string returned by the EDU0157 weather station.
 * based off bme280_example
 *
 * @author Kai Otsuka
 * @date 2026-7-7
 */

// stdlib includes
#include <stdio.h>
#include <string.h>

// system includes
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "stm32_systime.h"
#include "sys_app.h"
#include "usart.h"

extern I2C_HandleTypeDef hi2c1;

void I2C_Scan(void) {
  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Scanning I2C bus...\r\n");

  uint8_t found = 0;

  for (uint8_t addr = 1; addr < 128; addr++) {
    HAL_StatusTypeDef result;

    result = HAL_I2C_IsDeviceReady(&hi2c1,
                                   addr << 1,  // HAL expects 8-bit address
                                   3,          // retries
                                   100         // timeout (ms)
    );

    if (result == HAL_OK) {
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Found device at 0x%02X\r\n", addr);
      found++;
    }
  }

  if (found == 0) {
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "No I2C devices found.\r\n");
  } else {
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Scan complete. %d device(s) found.\r\n",
            found);
  }
}

// user includes
#include "EDU0157_sensor.h"

#ifndef DELAY
#define DELAY 1000
#endif

HAL_StatusTypeDef rc;

int main(void) {
  /* Reset peripherals and initialize HAL */
  HAL_Init();

  /* Configure system clock */
  SystemClock_Config();

  /* Initialize system app */
  SystemApp_Init();

  /* Initialize peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "HELLO\r\n");
  I2C_Scan();
  /* Initialize EDU0157 sensor */
  if (!EDU0157Init()) {
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "EDU0157 init failed\r\n");

    while (1);
  }

  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Example EDU0157 (%s), compile on %s %s\r\n",
          __FILE__, __DATE__, __TIME__);

  while (1) {
    HAL_Delay(DELAY);
    EDU0157Data data = {};

    int status = EDU0157MeasureAll(&data);
    if (status == 1) {
      const char *dir_str[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
      APP_LOG(
          TS_OFF, VLEVEL_ALWAYS,
          "WindSpeed: %.2f m/s, WindDirection: %s, Altitude: %.2f m, "
          "Pressure: %.2f hPa, Temperature: %.2f C, Humidity: %.2f %%RH\r\n",
          data.wind_speed,
          (data.wind_direction >= 0 && data.wind_direction < 8)
              ? dir_str[data.wind_direction]
              : "UNKNOWN",
          data.altitude, data.pressure, data.temperature, data.humidity);
    } else {
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "failed to read \r\n");
    }
  }
}