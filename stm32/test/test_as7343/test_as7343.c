/**
 * @file test_as7343.c
 * @brief Tests the ads library
 */

#include <stdio.h>
#include <unity.h>

#include "as7343.h"
#include "board.h"
#include "fifo.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

/**
 * @brief Setup code that runs at the start of every test
 */
void setUp(void) {}

/**
 * @brief Tear down code that runs at the end of every test
 */
void tearDown(void) {}

void test_init(void) {
  int8_t result = AS7343Init();
  TEST_ASSERT_EQUAL(result, SUCCESS);
}

void test_measure_spectral_data(void) {
  //tests readings on channel 0 TODO: Add more channels to the test  
  AS7343Data sensorReadings = {};
  
  AS7343GetMeasurement(&sensorReadings);

  TEST_ASSERT_GREATER_THAN(0, sensorReadings.channelCombined[0]);
}

/**
 * @brief Entry point for protobuf test
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  SystemApp_Init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();

  // wait for UART
  WaitForSerial();

  // Unit testing
  UNITY_BEGIN();

  // Tests for timestamp
  RUN_TEST(test_init);
  RUN_TEST(test_measure_spectral_data);

  UNITY_END();
}
