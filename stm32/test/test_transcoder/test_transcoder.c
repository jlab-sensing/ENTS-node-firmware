/**
 * @file test_transcoder.c
 *
 * @brief Tests encoding/decoding against binary data obtained from python
 *
 * Encoding of a Measurement message is compared against serialized data from
 * python using the same input parameters. A byte wise comparison is used for
 * these tests.
 *
 * Decoding a Response message takes binary data from python and compares the
 * decoded ResponseType to the expected. Both a SUCCESS and ERROR are checked to
 * cover the default case where "no" data is passed into the function.
 *
 * The parameters used for Measurement and Response messages are located in
 * generate_bytes.py and can be used to generate C code to copy into this test
 * file.
 *
 * @see generate_bytes.py
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2024-01-05
 */

#include <stdio.h>
#include <unity.h>

#include "gpio.h"
#include "main.h"
#include "main_helper.h"
#include "transcoder.h"
#include "usart.h"

/**
 * @brief Generated from CubeMX
 */
void SystemClock_Config(void);

/**
 * @brief Setup code that runs at the start of every test
 */
void setUp(void) {}

/**
 * @brief Tear down code that runs at the end of every test
 */
void tearDown(void) {}

void TestEncodePower(void) {
  uint8_t buffer[256];
  size_t buffer_len;

  buffer_len = EncodePowerMeasurement(1436079600, 7, 4, 37.13, 185.29, buffer);

  uint8_t data[] = {0xa,  0xa,  0x8,  0x4,  0x10, 0x7,  0x18, 0xf0,
                    0xab, 0xe3, 0xac, 0x5,  0x12, 0x12, 0x11, 0x71,
                    0x3d, 0xa,  0xd7, 0xa3, 0x90, 0x42, 0x40, 0x19,
                    0xe1, 0x7a, 0x14, 0xae, 0x47, 0x29, 0x67, 0x40};
  size_t data_len = 32;

  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, buffer, buffer_len);
  TEST_ASSERT_EQUAL_INT(data_len, buffer_len);
}

void TestEncodeTeros12(void) {
  uint8_t buffer[256];
  size_t buffer_len;

  buffer_len = EncodeTeros12Measurement(1436079600, 7, 4, 2124.62, 0.43, 24.8,
                                        123, buffer);

  uint8_t data[] = {0xa,  0xa,  0x8,  0x4,  0x10, 0x7,  0x18, 0xf0, 0xab,
                    0xe3, 0xac, 0x5,  0x1a, 0x1d, 0x11, 0xa,  0xd7, 0xa3,
                    0x70, 0x3d, 0x99, 0xa0, 0x40, 0x19, 0x85, 0xeb, 0x51,
                    0xb8, 0x1e, 0x85, 0xdb, 0x3f, 0x21, 0xcd, 0xcc, 0xcc,
                    0xcc, 0xcc, 0xcc, 0x38, 0x40, 0x28, 0x7b};
  size_t data_len = 43;

  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, buffer, buffer_len);
  TEST_ASSERT_EQUAL_INT(data_len, buffer_len);
}

void TestEncodePhytos31(void) {
  uint8_t buffer[256];
  size_t buffer_len;

  buffer_len =
      EncodePhytos31Measurement(1436079600, 7, 4, 1425.12, 1962.2, buffer);

  uint8_t data[] = {0xa,  0xa,  0x8,  0x4,  0x10, 0x7,  0x18, 0xf0,
                    0xab, 0xe3, 0xac, 0x5,  0x22, 0x12, 0x9,  0x14,
                    0xae, 0x47, 0xe1, 0x7a, 0x44, 0x96, 0x40, 0x11,
                    0xcd, 0xcc, 0xcc, 0xcc, 0xcc, 0xa8, 0x9e, 0x40};
  size_t data_len = 32;

  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, buffer, buffer_len);
  TEST_ASSERT_EQUAL_INT(data_len, buffer_len);
}

void TestEncodeBME280(void) {
  uint8_t buffer[256];
  size_t buffer_len;

  buffer_len =
      EncodeBME280Measurement(1436079600, 7, 4, 98473, 2275, 43600, buffer);

  uint8_t data[] = {0xa,  0xa,  0x8,  0x4,  0x10, 0x7,  0x18, 0xf0, 0xab,
                    0xe3, 0xac, 0x5,  0x2a, 0xb,  0x8,  0xa9, 0x81, 0x6,
                    0x10, 0xe3, 0x11, 0x18, 0xd0, 0xd4, 0x2};
  size_t data_len = 25;

  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, buffer, buffer_len);
  TEST_ASSERT_EQUAL_INT(data_len, buffer_len);
}

void TestDecodeResponseSuccess(void) {
  uint8_t data[] = {};
  size_t data_len = 0;

  Response_ResponseType resp_type = DecodeResponse(data, data_len);

  TEST_ASSERT_EQUAL(Response_ResponseType_SUCCESS, resp_type);
}

void TestDecodeResponseError(void) {
  uint8_t data[] = {0x8, 0x1};
  size_t data_len = 2;

  Response_ResponseType resp_type = DecodeResponse(data, data_len);

  TEST_ASSERT_EQUAL(Response_ResponseType_ERROR, resp_type);
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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();

  // wait for UART
  for (int i = 0; i < 1000000; i++) {
    __NOP();
  }

  // Unit testing
  UNITY_BEGIN();

  RUN_TEST(TestEncodePower);
  RUN_TEST(TestEncodeTeros12);
  RUN_TEST(TestEncodePhytos31);
  RUN_TEST(TestEncodeBME280);
  RUN_TEST(TestDecodeResponseSuccess);
  RUN_TEST(TestDecodeResponseError);

  UNITY_END();
}
