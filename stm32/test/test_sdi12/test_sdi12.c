/**
 * @file test_fifo.c
 * @brief Tests the fifo circular buffer library
 */

#include <stdio.h>
#include <unity.h>

#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "sdi12.h"
#include "usart.h"

void setUp(void) {}

void tearDown(void) {}

void test_SDI12_SendCommand_success(void) {
  SDI12Status status;
  // Command to request measurement ("0!\r\n" for example)
  char reqMeas[30];
  // Construct a command to request a measurement
  uint8_t size = snprintf(reqMeas, sizeof(reqMeas), "0M!");
  status = SDI12SendCommand(reqMeas, size);

  TEST_ASSERT_EQUAL(SDI12_OK, status);
}

void test_SDI12_Read_success(void) { TEST_IGNORE(); }

void test_SDI12_GetMeasurment_success(void) {
  SDI12Status status;
  uint8_t addr = '0';
  SDI12_Measure_TypeDef measurment_info;
  char buffer[20];
  status = SDI12GetMeasurment(addr, &measurment_info, buffer, 1000);

  TEST_ASSERT_EQUAL(SDI12_OK, status);
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  // wait for UART
  WaitForSerial();

  UNITY_BEGIN();
  RUN_TEST(test_SDI12_SendCommand_success);
  RUN_TEST(test_SDI12_GetMeasurment_success);
  UNITY_END();
  /* USER CODE END 3 */
}
