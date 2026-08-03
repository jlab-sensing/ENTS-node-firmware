/**
 * @example example_lora_downlink.c
 *
 * Prints downlink information if one was scheduled before a periodic uplink
 * was sent.
 *
 * @author Eric Tran
 * @date 2026-8-3
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
#include "lora_downlink.h"
#include "app_lorawan.h"
#include "sensors.h"

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

  // init lorawan
  BME280Init();
  SensorsAdd(BME280Measure);
  MX_LoRaWAN_Init();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS, "Example Lora Downlink (%s), compile on %s %s\r\n",
          __FILE__, __DATE__, __TIME__);

  // Infinite loop
  while (1) {
    // Sleep
    // HAL_Delay(DELAY);

    // BME280Data data = {};

    // BME280Status status = BME280MeasureAll(&data);
    // if (status != BME280_STATUS_OK) {
    //   APP_LOG(TS_OFF, VLEVEL_ALWAYS, "BME280 Error, status: %d\r\n", status);
    //   continue;
    // }
    MX_LoRaWAN_Process();

  //   // check if radio is busy
  // if (LmHandlerIsBusy()) {
  //   APP_LOG(TS_ON, VLEVEL_M, "LmHandler is busy\r\n");
  //   return;
  // }

  // // check if buffer is empty
  // if (FramBufferLen() <= 0) {
  //   APP_LOG(TS_ON, VLEVEL_M, "Nothing in buffer\r\n");
  //   // return;
  // }

  
  // LmHandlerSend(&AppData, LORAWAN_DEFAULT_CONFIRMED_MSG_STATE, false);

    // APP_LOG(TS_OFF, VLEVEL_ALWAYS,
    //         "Pressure: %u, Temperature: %d, Humidity: %u\r\n", data.pressure,
    //         data.temperature, data.humidity);
    if(downlinkIsNewDataReady())
    {
      APP_LOG(TS_ON, VLEVEL_M, "HAAAAAAAAAAAAAAAAAAngnwieugtnw4gnwgnw4gnw4j\r\n");
    }
    
  }
}
