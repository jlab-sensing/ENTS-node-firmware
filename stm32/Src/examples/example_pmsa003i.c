/**
 * @example example_pmsa003i.c
 *
 * Prints output of pmsa003i air quality sensor
 * Modified version of example_bme280 by John Madden
 *
 * @author Kai Otsuka
 * @date 2026-7-7
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
#include "pmsa003i.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 2000
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

  // Infinite loop
  while (1) {
    //sleep
    HAL_Delay(DELAY);

    pmsa003i_data_t data = {};

    bool status = pmsa003i_read(&data);

    if (status) {
        //format same as arduino library: https://learn.adafruit.com/pmsa003i/arduino
      APP_LOG(TS_OFF, VLEVEL_ALWAYS,
              "---------------------------------------\r\n");
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Concentration Units (standard)\r\n");
      APP_LOG(TS_OFF, VLEVEL_ALWAYS,
              "PM 1.0: %u\t\tPM 2.5: %u\t\tPM 10: %u\r\n", data.pm10_standard,
              data.pm25_standard, data.pm100_standard);

      APP_LOG(TS_OFF, VLEVEL_ALWAYS,
              "---------------------------------------\r\n");
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Concentration Units (environmental)\r\n");
      APP_LOG(TS_OFF, VLEVEL_ALWAYS,
              "PM 1.0: %u\t\tPM 2.5: %u\t\tPM 10: %u\r\n", data.pm10_env,
              data.pm25_env, data.pm100_env);

      APP_LOG(TS_OFF, VLEVEL_ALWAYS,
              "---------------------------------------\r\n");
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Particles > 0.3um / 0.1L air:%u\r\n",
              data.particles_03um);
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Particles > 0.5um / 0.1L air:%u\r\n",
              data.particles_05um);
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Particles > 1.0um / 0.1L air:%u\r\n",
              data.particles_10um);
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Particles > 2.5um / 0.1L air:%u\r\n",
              data.particles_25um);
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Particles > 5.0um / 0.1L air:%u\r\n",
              data.particles_50um);
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Particles > 10 um / 0.1L air:%u\r\n",
              data.particles_100um);

      APP_LOG(TS_OFF, VLEVEL_ALWAYS,
              "---------------------------------------\r\n");
    } else {
      APP_LOG(TS_OFF, VLEVEL_ALWAYS, "failed to read\r\n");
    }
  }
}
