/**
 * @file test_battery.c
 * @brief unit test for battery.c
 *
 * @author Ahmed Falah <ahmed.falah600@gmail.com>
 * @date 2024-03-18
 */

#include <stdio.h>
#include <unity.h>

#include "battery.h"
#include "board.h"
#include "gpio.h"
#include "main.h"
#include "usart.h"

void SystemClock_Config(void);

/**
 * @brief Setup code that runs at the start of every test
 */
void setUp(void) {}

/**
 * @brief Tear down code that runs at the end of every test
 */
void tearDown(void) {}

// Test case for battery_init
void test_battery_init_success(void) {
  HAL_StatusTypeDef ret = battery_init();
  // Assert successful initialization
  TEST_ASSERT_EQUAL(HAL_OK, ret);
  /* We can add conditions based on the return status of the battery_init()*/
}

// Test case for battery_voltage valid reading
void test_battery_voltage_stability(void) {
  // Threshold for voltage change
  const int MAX_VOLTAGE_CHANGE = 100;
  // Calling battery_voltage() function multiple times with delay between
  // readings
  unsigned int voltage1 = battery_voltage();
  HAL_Delay(1000);  // Delay 1sec
  unsigned int voltage2 = battery_voltage();

  // Assert that the change in voltage is within the threshold
  TEST_ASSERT_INT_WITHIN_MESSAGE(
      -MAX_VOLTAGE_CHANGE, MAX_VOLTAGE_CHANGE, voltage2 - voltage1,
      "Battery voltage changed excessively between readings.");
}

// void test_battery_voltage_usb_power(void) {
//   // Take multiple readings and store them in an array
//   unsigned int voltages[5];
//   for (int i = 0; i < 5; i++) {
//     voltages[i] = battery_voltage();
//     HAL_Delay(100);
//   }
//   // Calculate average voltage
//   unsigned int average_voltage = 0;
//   for (int i = 0; i < 5; i++) {
//     average_voltage += voltages[i];
//   }
//   average_voltage /= 5;
//   // Expected voltage range for USB power
//   const int EXPECTED_USB_VOLTAGE = (VOLTAGE_REF * 1000) / FULL_SCALE; // mV
//   const int USB_VOLTAGE_TOLERANCE = 200;
//   // Assert that the average voltage is within the expected range
//   TEST_ASSERT_INT_WITHIN_MESSAGE(EXPECTED_USB_VOLTAGE -
//   USB_VOLTAGE_TOLERANCE,
//                                  EXPECTED_USB_VOLTAGE +
//                                  USB_VOLTAGE_TOLERANCE, average_voltage,
//                                  "Battery voltage under USB power is outside
//                                  expected range.");
// }

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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  // wait for UART
  WaitForSerial();

  // Unit testing
  UNITY_BEGIN();
  // Run all the tests
  RUN_TEST(test_battery_init_success);
  RUN_TEST(test_battery_voltage_stability);
  // RUN_TEST(test_battery_voltage_usb_power);
  UNITY_END();
}
