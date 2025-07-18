/**
 * @file test_proto.c
 * @brief Tests ability to encode and decode protobuf messages
 *
 * Encoding/decoding of each message type is tested to ensure functionality of
 * nanopb and correctly matched tests with generate protobuf source files.
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-27
 */

#include <stdio.h>
#include <unity.h>

#include "board.h"
#include "gpio.h"
#include "main.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "soil_power_sensor.pb.h"
#include "usart.h"

#ifndef UNIX_EPOCHS
/** Unix epochs encoded in timestamp. Default corresponds to Mon Nov 27 2023
 * 03:25:19 GMT+0000 */
#define UNIX_EPOCHS 1701055519
#endif /* UNIX_EPOCHS */

#ifndef FLOAT_DELTA
/** Delta between comparison between floating point numbers */
#define FLOAT_DELTA 1e-4
#endif /* FLOAT_DELTA */

/** Buffer for encoded data */
uint8_t buffer[256];
/** message length */
size_t msg_len;

/** Nanopb output buffer */
pb_ostream_t ostream;
/** Nanopb input buffer */
pb_istream_t istream;

/** Nanopb status code */
bool status;

/**
 * @brief Default MeasurementMetadata
 *
 * Uses as submessage when testing parent messages. Defaults to zeros with
 * ts_default
 *
 * @see ts_default
 */
MeasurementMetadata meta_default;

/**
 * @brief Setup code that runs at the start of every test
 *
 * Creates required variables that are shared between all tests. The output
 * buffer is initialized based on length of the buffer. Input buffer is defined,
 * but not initialized since it depends on the message length. Default values
 * are defined for Timestamp and MeasurementMetadata.
 */
void setUp(void) {
  // create output buffer
  ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  // Set default measurement metadata
  meta_default.ts = UNIX_EPOCHS;
  meta_default.cell_id = 0;
  meta_default.logger_id = 0;
}

/**
 * @brief Tear down code that runs at the end of every test
 */
void tearDown(void) {}

void test_meta(void) {
  MeasurementMetadata meta_encode = MeasurementMetadata_init_zero;

  meta_encode.ts = UNIX_EPOCHS;
  meta_encode.cell_id = 0;
  meta_encode.logger_id = 1;

  status = pb_encode(&ostream, MeasurementMetadata_fields, &meta_encode);
  msg_len = ostream.bytes_written;

  TEST_ASSERT_TRUE(status);
  TEST_ASSERT_GREATER_THAN(0, msg_len);

  MeasurementMetadata meta_decode;

  istream = pb_istream_from_buffer(buffer, msg_len);
  status = pb_decode(&istream, MeasurementMetadata_fields, &meta_decode);

  TEST_ASSERT_TRUE(status);

  TEST_ASSERT_EQUAL(0, meta_decode.cell_id);
  TEST_ASSERT_EQUAL(1, meta_decode.logger_id);

  TEST_ASSERT_EQUAL(UNIX_EPOCHS, meta_decode.ts);
}

void test_power(void) {
  Measurement power_encode = Measurement_init_zero;
  power_encode.which_measurement = Measurement_power_tag;
  power_encode.has_meta = true;
  power_encode.meta = meta_default;
  power_encode.measurement.power.voltage = 1.2;
  power_encode.measurement.power.current = 120.1;

  status = pb_encode(&ostream, Measurement_fields, &power_encode);
  msg_len = ostream.bytes_written;

  TEST_ASSERT_TRUE(status);
  TEST_ASSERT_GREATER_THAN(0, msg_len);

  Measurement power_decode;

  istream = pb_istream_from_buffer(buffer, msg_len);
  status = pb_decode(&istream, Measurement_fields, &power_decode);

  TEST_ASSERT_TRUE(status);

  // Meta
  TEST_ASSERT_TRUE(power_decode.has_meta);
  TEST_ASSERT_EQUAL(0, power_decode.meta.cell_id);
  TEST_ASSERT_EQUAL(0, power_decode.meta.logger_id);
  // Timestamp
  TEST_ASSERT_EQUAL(UNIX_EPOCHS, power_decode.meta.ts);

  TEST_ASSERT_EQUAL(Measurement_power_tag, power_decode.which_measurement);
  TEST_ASSERT_DOUBLE_WITHIN(FLOAT_DELTA, 1.2,
                            power_decode.measurement.power.voltage);
  TEST_ASSERT_DOUBLE_WITHIN(FLOAT_DELTA, 120.1,
                            power_decode.measurement.power.current);
}

void test_teros12(void) {
  Measurement teros12_encode = Measurement_init_zero;
  teros12_encode.which_measurement = Measurement_teros12_tag;
  teros12_encode.has_meta = true;
  teros12_encode.meta = meta_default;
  teros12_encode.measurement.teros12.ec = 200;
  teros12_encode.measurement.teros12.temp = 24.5;
  teros12_encode.measurement.teros12.vwc_adj = 32.4;
  teros12_encode.measurement.teros12.vwc_raw = 3200.2;

  status = pb_encode(&ostream, Measurement_fields, &teros12_encode);
  msg_len = ostream.bytes_written;

  TEST_ASSERT_TRUE(status);
  TEST_ASSERT_GREATER_THAN(0, msg_len);

  Measurement teros12_decode;

  istream = pb_istream_from_buffer(buffer, msg_len);
  status = pb_decode(&istream, Measurement_fields, &teros12_decode);

  TEST_ASSERT_TRUE(status);

  // Meta
  TEST_ASSERT_TRUE(teros12_decode.has_meta);
  TEST_ASSERT_EQUAL(0, teros12_decode.meta.cell_id);
  TEST_ASSERT_EQUAL(0, teros12_decode.meta.logger_id);
  // Timestamp
  TEST_ASSERT_EQUAL(UNIX_EPOCHS, teros12_decode.meta.ts);

  TEST_ASSERT_EQUAL(Measurement_teros12_tag, teros12_decode.which_measurement);
  TEST_ASSERT_EQUAL(200, teros12_decode.measurement.teros12.ec);
  TEST_ASSERT_DOUBLE_WITHIN(FLOAT_DELTA, 24.5,
                            teros12_decode.measurement.teros12.temp);
  TEST_ASSERT_DOUBLE_WITHIN(FLOAT_DELTA, 32.4,
                            teros12_decode.measurement.teros12.vwc_adj);
  TEST_ASSERT_DOUBLE_WITHIN(FLOAT_DELTA, 3200.2,
                            teros12_decode.measurement.teros12.vwc_raw);
}

void test_bme280(void) {
  Measurement bme280_encode = Measurement_init_zero;
  bme280_encode.which_measurement = Measurement_bme280_tag;
  bme280_encode.has_meta = true;
  bme280_encode.meta = meta_default;
  bme280_encode.measurement.bme280.temperature = 27;
  bme280_encode.measurement.bme280.pressure = 300;
  bme280_encode.measurement.bme280.humidity = 10;

  status = pb_encode(&ostream, Measurement_fields, &bme280_encode);
  msg_len = ostream.bytes_written;

  TEST_ASSERT_TRUE(status);
  TEST_ASSERT_GREATER_THAN(0, msg_len);

  Measurement bme280_decode;

  istream = pb_istream_from_buffer(buffer, msg_len);
  status = pb_decode(&istream, Measurement_fields, &bme280_decode);

  TEST_ASSERT_TRUE(status);

  // Meta
  TEST_ASSERT_TRUE(bme280_decode.has_meta);
  TEST_ASSERT_EQUAL(0, bme280_decode.meta.cell_id);
  TEST_ASSERT_EQUAL(0, bme280_decode.meta.logger_id);
  // Timestamp
  TEST_ASSERT_EQUAL(UNIX_EPOCHS, bme280_decode.meta.ts);

  TEST_ASSERT_EQUAL(Measurement_bme280_tag, bme280_decode.which_measurement);
  TEST_ASSERT_EQUAL(27, bme280_decode.measurement.bme280.temperature);
  TEST_ASSERT_EQUAL(300, bme280_decode.measurement.bme280.pressure);
  TEST_ASSERT_EQUAL(10, bme280_decode.measurement.bme280.humidity);
}

void test_response(void) {
  Response response_encode = Response_init_zero;
  response_encode.resp = Response_ResponseType_SUCCESS;

  status = pb_encode(&ostream, Response_fields, &response_encode);
  msg_len = ostream.bytes_written;

  TEST_ASSERT_TRUE(status);

  Response response_decode;

  istream = pb_istream_from_buffer(buffer, msg_len);
  status = pb_decode(&istream, Response_fields, &response_decode);

  TEST_ASSERT_TRUE(status);

  TEST_ASSERT_EQUAL(Response_ResponseType_SUCCESS, response_decode.resp);
}

void test_esp32_wifi() {
  char ssid[] = "Hello";
  char passwd[] = "World";
  char url[] = "http://test.com";
  uint8_t bytes[] = {0, 1, 2, 3, 4};

  // encoding
  Esp32Command encode = Esp32Command_init_zero;
  encode.which_command = Esp32Command_wifi_command_tag;
  encode.command.wifi_command.type = WiFiCommand_Type_CONNECT;
  strncpy(encode.command.wifi_command.ssid, ssid, sizeof(ssid));
  strncpy(encode.command.wifi_command.passwd, passwd, sizeof(passwd));
  strncpy(encode.command.wifi_command.url, url, sizeof(url));
  encode.command.wifi_command.rc = 200;
  encode.command.wifi_command.ts = 1600000;
  memcpy(encode.command.wifi_command.resp.bytes, bytes, sizeof(bytes));
  encode.command.wifi_command.resp.size = 5;
  encode.command.wifi_command.port = 443;

  status = pb_encode(&ostream, Esp32Command_fields, &encode);
  msg_len = ostream.bytes_written;

  TEST_ASSERT_TRUE(status);
  TEST_ASSERT_GREATER_THAN(0, msg_len);

  // decoding
  Esp32Command decode = Esp32Command_init_zero;
  istream = pb_istream_from_buffer(buffer, msg_len);
  status = pb_decode(&istream, Esp32Command_fields, &decode);

  TEST_ASSERT_TRUE(status);

  TEST_ASSERT_EQUAL(Esp32Command_wifi_command_tag, decode.which_command);
  TEST_ASSERT_EQUAL(WiFiCommand_Type_CONNECT, decode.command.wifi_command.type);
  TEST_ASSERT_EQUAL_STRING(ssid, decode.command.wifi_command.ssid);
  TEST_ASSERT_EQUAL_STRING(passwd, decode.command.wifi_command.passwd);
  TEST_ASSERT_EQUAL_STRING(url, decode.command.wifi_command.url);
  TEST_ASSERT_EQUAL(200, decode.command.wifi_command.rc);
  TEST_ASSERT_EQUAL(1600000, decode.command.wifi_command.ts);
  TEST_ASSERT_EQUAL_INT8_ARRAY(bytes, decode.command.wifi_command.resp.bytes,
                               sizeof(bytes));
  TEST_ASSERT_EQUAL(5, decode.command.wifi_command.resp.size);
  TEST_ASSERT_EQUAL(443, decode.command.wifi_command.port);
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

  // Tests for MeasurementMetadata
  RUN_TEST(test_meta);
  // Tests for PowerMeasurement
  RUN_TEST(test_power);
  // Tests for Teros12
  RUN_TEST(test_teros12);
  // Tests for Response
  RUN_TEST(test_response);
  // Tests Esp32Command WiFI
  RUN_TEST(test_esp32_wifi);

  UNITY_END();
}
