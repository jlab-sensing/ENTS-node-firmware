/**
 * @file test_template.c
 * @brief Tests for the BME280 environmental sensor
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2024-01-05
 *
 * Before running the tests, a BME280 sensor should be connected to the device.
 * The tests look at initialization, reading each channel and formatting of
 * measurement data. Values are checked such that they appears within the
 * operating range of the sensor.
 */

#include <stdio.h>
#include <unity.h>

#include "bme280_sensor.h"
#include "board.h"
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
  BME280Status status = BME280Init();
  TEST_ASSERT_EQUAL(BME280_STATUS_OK, status);
}

void test_measure_temperature(void) {
  BME280Data data;
  BME280Status status = BME280MeasureAll(&data);

  TEST_ASSERT_EQUAL(BME280_STATUS_OK, status);

  TEST_ASSERT_GREATER_THAN(0, data.temperature);
  TEST_ASSERT_LESS_THAN(6500, data.temperature);
}

void test_measure_pressure(void) {
  BME280Data data;
  BME280Status status = BME280MeasureAll(&data);

  TEST_ASSERT_EQUAL(BME280_STATUS_OK, status);

  TEST_ASSERT_GREATER_THAN(3000, data.pressure);
  TEST_ASSERT_LESS_THAN(11000, data.temperature);
}

void test_measure_humidity(void) {
  BME280Data data;
  BME280Status status = BME280MeasureAll(&data);

  TEST_ASSERT_EQUAL(BME280_STATUS_OK, status);

  TEST_ASSERT_GREATER_THAN(0, data.temperature);
  TEST_ASSERT_LESS_THAN(100000, data.temperature);
}

void test_measure(void) {
  uint8_t buffer[256];
  size_t buffer_len = 0;

  buffer_len = BME280Measure(buffer);

  TEST_ASSERT_GREATER_THAN(0, buffer_len);
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
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  // wait for UART
  WaitForSerial();

  // Unit testing
  UNITY_BEGIN();

  // Tests for timestamp
  RUN_TEST(test_init);
  RUN_TEST(test_measure_temperature);
  RUN_TEST(test_measure_pressure);
  RUN_TEST(test_measure_humidity);
  RUN_TEST(test_measure);

  UNITY_END();
}
