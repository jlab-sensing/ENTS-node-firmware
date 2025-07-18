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

#include "board.h"
#include "gpio.h"
#include "main.h"
#include "usart.h"

#include "transcoder.h"

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

void TestEncodeTeros21(void) {
  uint8_t buffer[256];
  size_t buffer_len;

  buffer_len =
      EncodeTeros21Measurement(1737671549, 51, 12, 134.5, 22.4, buffer);

  uint8_t data[] = {0xa,  0xa,  0x8,  0xc,  0x10, 0x33, 0x18, 0xfd,
                    0x86, 0xcb, 0xbc, 0x6,  0x32, 0x12, 0x9,  0x0,
                    0x0,  0x0,  0x0,  0x0,  0xd0, 0x60, 0x40, 0x11,
                    0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x36, 0x40};
  size_t data_len = 32;

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
 * Command used to generate the code for validating the output
 *
 * sps encode --c esp32command wifi --ssid Hello --passwd World --url
 * "http://www.test.com/" --port 443 --rc 200 --ts 1600000 CONNECT
 */
void TestEncodeWiFi() {
  uint8_t buffer[WiFiCommand_size];
  size_t buffer_len;

  char hello[] = "Hello";
  char world[] = "World";
  char url[] = "http://www.test.com/";

  WiFiCommand wifi_cmd = WiFiCommand_init_zero;
  wifi_cmd.type = WiFiCommand_Type_CONNECT;
  strncpy(wifi_cmd.ssid, hello, sizeof(wifi_cmd.ssid));
  strncpy(wifi_cmd.passwd, world, sizeof(wifi_cmd.passwd));
  strncpy(wifi_cmd.url, url, sizeof(wifi_cmd.url));
  wifi_cmd.port = 443;
  wifi_cmd.rc = 200;
  wifi_cmd.ts = 1600000;
  // WiFiCommand.resp is left NULL

  buffer_len = EncodeWiFiCommand(&wifi_cmd, buffer, sizeof(buffer));

  uint8_t data[] = {0x1a, 0x2e, 0x12, 0x5,  0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x1a,
                    0x5,  0x57, 0x6f, 0x72, 0x6c, 0x64, 0x22, 0x14, 0x68, 0x74,
                    0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x74,
                    0x65, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x28, 0xc8,
                    0x1,  0x30, 0x80, 0xd4, 0x61, 0x40, 0xbb, 0x3};
  size_t data_len = 48;

  TEST_ASSERT_EQUAL(data_len, buffer_len);
  TEST_ASSERT_EQUAL_HEX8_ARRAY(data, buffer, buffer_len);
}

/**
 * Command used to generate the code for validating the output
 *
 * sps encode --c esp32command wifi --ssid Hello --passwd World --url
 * "http://www.test.com/" --port 443 --rc 200 --ts 1600000 CONNECT
 */
void TestDecodeWiFi() {
  uint8_t data[] = {0x1a, 0x2e, 0x12, 0x5,  0x48, 0x65, 0x6c, 0x6c, 0x6f, 0x1a,
                    0x5,  0x57, 0x6f, 0x72, 0x6c, 0x64, 0x22, 0x14, 0x68, 0x74,
                    0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e, 0x74,
                    0x65, 0x73, 0x74, 0x2e, 0x63, 0x6f, 0x6d, 0x2f, 0x28, 0xc8,
                    0x1,  0x30, 0x80, 0xd4, 0x61, 0x40, 0xbb, 0x3};
  size_t data_len = 48;

  Esp32Command cmd = DecodeEsp32Command(data, data_len);

  char ssid[] = "Hello";
  char passwd[] = "World";
  char url[] = "http://www.test.com/";

  TEST_ASSERT_EQUAL(Esp32Command_wifi_command_tag, cmd.which_command);
  TEST_ASSERT_EQUAL_STRING(ssid, cmd.command.wifi_command.ssid);
  TEST_ASSERT_EQUAL_STRING(passwd, cmd.command.wifi_command.passwd);
  TEST_ASSERT_EQUAL_STRING(url, cmd.command.wifi_command.url);
  TEST_ASSERT_EQUAL(443, cmd.command.wifi_command.port);
  TEST_ASSERT_EQUAL(200, cmd.command.wifi_command.rc);
  TEST_ASSERT_EQUAL(1600000, cmd.command.wifi_command.ts);
  // bytes is not checked since statically allocated
  TEST_ASSERT_EQUAL(0, cmd.command.wifi_command.resp.size);
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
  WaitForSerial();

  // Unit testing
  UNITY_BEGIN();

  RUN_TEST(TestEncodePower);
  RUN_TEST(TestEncodeTeros12);
  RUN_TEST(TestEncodePhytos31);
  RUN_TEST(TestEncodeBME280);
  RUN_TEST(TestEncodeTeros21);
  RUN_TEST(TestDecodeResponseSuccess);
  RUN_TEST(TestDecodeResponseError);
  RUN_TEST(TestEncodeWiFi);
  RUN_TEST(TestDecodeWiFi);

  UNITY_END();
}
