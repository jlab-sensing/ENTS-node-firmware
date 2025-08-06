// Copyright 2023 UCSC

/**
 * @file test_fram.c
 * @brief Tests the fram library
 *
 * Test code exists but not enabled to run tests across multiple pages. The
 * newest hardware version of the board (MB85RC1MT) only has a single page,
 * therefore testing is not possible.
 */

#include <stdio.h>
#include <unity.h>

#include "board.h"
#include "fram.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "usart.h"

void setUp(void) {}

/**
 * @brief Tear down code that runs at the end of every test
 */
void tearDown(void) {}

void test_FramWrite_ValidData(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramAddr addr = 0x00;

  FramStatus status = FramWrite(addr, data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramWrite_OutOfRange(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramAddr addr = FramSize() + 1;  // Address out of range

  FramStatus status = FramWrite(addr, data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OUT_OF_RANGE, status);
}

void test_FramWrite_ZeroLength(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramAddr addr = 0x00;

  FramStatus status = FramWrite(addr, data, 0);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramWrite_All(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramAddr addr = 0x00;

  while (addr + sizeof(data) < FramSize()) {
    FramStatus status = FramWrite(addr, data, sizeof(data));
    TEST_ASSERT_EQUAL(FRAM_OK, status);
    addr += sizeof(data);
  }
}

void test_FramRead_Status(void) {
  uint8_t data[5];
  FramAddr addr = 0x0;

  FramStatus status = FramRead(addr, sizeof(data), data);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramRead_ValidData(void) {
  FramAddr addr = 0x00;

  uint8_t write_data[] = {1, 2, 3, 4, 5};
  FramWrite(addr, write_data, sizeof(write_data));

  uint8_t read_data[5];
  FramStatus status = FramRead(addr, sizeof(read_data), read_data);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(write_data, read_data, 5);
}

void test_FramRead_ZeroLength(void) {
  uint8_t data[5];
  FramAddr addr = 0x0;

  FramStatus status = FramRead(addr, 0, data);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramRead_OutOfRange(void) {
  uint8_t data[5];
  FramAddr addr = FramSize() + 1;

  FramStatus status = FramRead(addr, sizeof(data), data);

  TEST_ASSERT_EQUAL(FRAM_OUT_OF_RANGE, status);
}

void test_FramRead_All(void) {
  FramAddr addr = 0x00;

  uint8_t write_data[] = {1, 2, 3, 4, 5};

  while (addr + sizeof(write_data) < FramSize()) {
    FramWrite(addr, write_data, sizeof(write_data));
    addr += sizeof(write_data);
  }

  uint8_t read_data[5];
  while (addr + sizeof(read_data) < FramSize()) {
    FramStatus status = FramRead(addr, sizeof(read_data), read_data);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(write_data, read_data, 5);
    addr += sizeof(read_data);
  }
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  // wait for UART
  WaitForSerial();

  UNITY_BEGIN();
  RUN_TEST(test_FramWrite_ValidData);
  RUN_TEST(test_FramWrite_ZeroLength);
  RUN_TEST(test_FramWrite_OutOfRange);
  RUN_TEST(test_FramWrite_All);
  RUN_TEST(test_FramRead_ValidData);
  RUN_TEST(test_FramRead_ZeroLength);
  RUN_TEST(test_FramRead_OutOfRange);
  RUN_TEST(test_FramRead_All);
  UNITY_END();
}
