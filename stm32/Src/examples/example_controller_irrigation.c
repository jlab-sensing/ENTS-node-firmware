/**
 * @example example_controller_irrigation.c
 *
 * Prints out the saved state from the webserver.
 *
 * @author John Madden
 * @date 2025-08-19
 */

// stdlib includes
#include <stdio.h>

// system includes
#include "board.h"
#include "controller/controller.h"
#include "controller/irrigation.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"
#include "soil_power_sensor.pb.h"

// user includes
#include "solenoid.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();

  // init system app
  SystemApp_Init();

  // init solenoid
  SolenoidInit();

  APP_LOG(TS_OFF, VLEVEL_ALWAYS,
          "Example controller irrigation (%s), compile on %s %s\r\n", __FILE__,
          __DATE__, __TIME__);

  ControllerInit();

  // Infinite loop
  while (1) {
    // Sleep
    HAL_Delay(1000);

    //HandleClinet();
    IrrigationCommand_State state = ControllerIrrigationCheck();
    APP_LOG(TS_ON, VLEVEL_ALWAYS, "Irrigation state: %d\r\n", state);
  }
}
