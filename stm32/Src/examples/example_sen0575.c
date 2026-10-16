/**
 * @example example_bme280.c
 *
 * Prints temperature, pressure, and humidity from the BME280 sensor on an
 * infinite loop.
 *
 * @author Eric Tran
 * @date 2026-10-13
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
#include "sen0575.h"

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

  // init bme280
//   sen0575Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Example sen0575 (%s), compile on %s %s\r\n",
          __FILE__, __DATE__, __TIME__);

  // Infinite loop
  while (1) {
    // Sleep
    HAL_Delay(DELAY);

    uint32_t data;
    data = sen0575RainfallRaw();
    APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Count: %d\r\n", data);
    // BME280Status status = BME280MeasureAll(&data);
    // if (status != BME280_STATUS_OK) {
    //   APP_LOG(TS_OFF, VLEVEL_ALWAYS, "BME280 Error, status: %d\r\n", status);
    //   continue;
    // }

    // APP_LOG(TS_OFF, VLEVEL_ALWAYS,
    //         "Pressure: %u, Temperature: %d, Humidity: %u\r\n", data.pressure,
    //         data.temperature, data.humidity);
  }
}
