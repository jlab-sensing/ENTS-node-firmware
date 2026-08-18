/**
 * @example example_bme280.c
 *
 * Prints temperature, pressure, and humidity from the BME280 sensor on an
 * infinite loop.
 *
 * @author John Madden
 * @date 2023-11-17
 */

// stdlib includes
#include <stdio.h>

// system includes
#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "bme280_sensor.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

/** Global variable for all return codes */
HAL_StatusTypeDef rc;

/**
 * @brief Entry point for battery test
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  // init system app
  SystemApp_Init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Example BME280 (%s), compile on %s %s\r\n",
          __FILE__, __DATE__, __TIME__);

  // init bme280
  BME280Status rslt = BME280Init();
  if (rslt != BME280_OK) {
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Failed to initialize BME280: %d\r\n", rslt);
  }

  // Infinite loop
  while (1) {
    // Sleep
    HAL_Delay(DELAY);

    BME280Data data = {};

    BME280Status status = BME280MeasureAll(&data);
    if (status != BME280_STATUS_OK) {
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "BME280 Error, status: %d\r\n", status);
      continue;
    }

    APP_LOG(TS_OFF, VLEVEL_ALWAYS,
            "Pressure: %u Pa, Temperature: %d * 0.01 deg C, Humidity: %u * "
            "0.001 %%RH\r\n",
            data.pressure, data.temperature, data.humidity);
  }
}
