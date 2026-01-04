/** 
 * @brief Tests protobuf sensor library.
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2025-12-10
 */


#include <stdio.h>
#include <unity.h>

#include "board.h"
#include "gpio.h"
#include "main.h"
#include "sensor.h"
#include "usart.h"

/**
 * @brief Setup code that runs at the start of every test
 */
void setUp(void) {}

/**
 * @brief Tear down code that runs at the end of every test
 */
void tearDown(void) {}




/**
 * @brief Encodes then decodes single sensor measurement.
 */
void TestTranscodeSensorMeasurement(void) {
  SensorStatus status = SENSOR_OK;
  
  uint8_t buffer[256];
  size_t buffer_len = sizeof(buffer);


  // encode
  SensorMeasurement in = SensorMeasurement_init_zero;

  in.has_meta = true;
  in.meta.ts = 123;
  in.meta.logger_id = 456;
  in.meta.cell_id = 789;

  in.type = SensorType_NONE;

  in.which_value = SensorMeasurement_unsigned_int_tag;
  in.value.unsigned_int = 9876543210ULL;

  status = EncodeSensorMeasurement(&in, buffer, &buffer_len);
  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_GREATER_THAN(0, buffer_len);


  // decode
  SensorMeasurement out = SensorMeasurement_init_zero;

  status = DecodeSensorMeasurement(buffer, buffer_len, &out);
  TEST_ASSERT_EQUAL(SENSOR_OK, status);

  // check values
  TEST_ASSERT_EQUAL(in.has_meta, out.has_meta);
  TEST_ASSERT_EQUAL(in.meta.ts, out.meta.ts);
  TEST_ASSERT_EQUAL(in.meta.logger_id, out.meta.logger_id);
  TEST_ASSERT_EQUAL(in.meta.cell_id, out.meta.cell_id);
  TEST_ASSERT_EQUAL(in.type, out.type);
  TEST_ASSERT_EQUAL(in.which_value, out.which_value);
  TEST_ASSERT_EQUAL(in.value.unsigned_int, out.value.unsigned_int);
}

/**
 * @brief Encodes then decodes repeated sensor measurements.
 */
void TestTranscodeRepeatedSensorMeasurements(void) {
  SensorStatus status = SENSOR_OK;
  uint8_t buffer[512];
  size_t buffer_len = 0;


  // encode
  SensorMeasurement in_array[10] = {};
  size_t len = 10;

  Metadata meta = Metadata_init_zero;
  meta.ts = 1000;
  meta.logger_id = 2000;
  meta.cell_id = 3000;

  for (int i = 0; i < len; i++) {
    SensorMeasurement* in = &in_array[i];

    in->has_meta = true;
    in->meta.ts = 123 + i;
    in->meta.logger_id = 456 + i;
    in->meta.cell_id = 789 + i;
    in->type = SensorType_NONE;

    in->which_value = SensorMeasurement_unsigned_int_tag;
    in->value.unsigned_int = 9876543210ULL + i;
  }

  status = EncodeRepeatedSensorMeasurements(meta, in_array, len, buffer, sizeof(buffer), &buffer_len);
  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_GREATER_THAN(0, buffer_len);



  // decode
  RepeatedSensorMeasurements rep_out = RepeatedSensorMeasurements_init_zero;

  status = DecodeRepeatedSensorMeasurements(buffer, buffer_len, &rep_out);
  
  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_EQUAL(10, rep_out.measurements_count);

  for (int i = 0; i < 10; i++) {
    SensorMeasurement* in = &in_array[i];
    SensorMeasurement* out = &rep_out.measurements[i];

    // check values
    TEST_ASSERT_EQUAL(in->has_meta, out->has_meta);
    TEST_ASSERT_EQUAL(in->meta.ts, out->meta.ts);
    TEST_ASSERT_EQUAL(in->meta.logger_id, out->meta.logger_id);
    TEST_ASSERT_EQUAL(in->meta.cell_id, out->meta.cell_id);
    TEST_ASSERT_EQUAL(in->type, out->type);
    TEST_ASSERT_EQUAL(in->which_value, out->which_value);
    TEST_ASSERT_EQUAL(in->value.unsigned_int, out->value.unsigned_int);
  }
}

void TestEncodeUint32Measurement(void) {
  SensorStatus status = SENSOR_OK;

  Metadata meta = Metadata_init_zero;
  meta.ts = 1111;
  meta.logger_id = 2222;
  meta.cell_id = 3333;
  
  uint64_t value = 1234567890123456789ULL;

  uint8_t buffer[256];
  size_t buffer_len = sizeof(buffer);
  status = EncodeUint32Measurement(meta, value, SensorType_POWER_VOLTAGE, 
                              buffer, &buffer_len);

  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_GREATER_THAN(0, buffer_len);
}


void TestEncodeInt32Measurement(void) {
  SensorStatus status = SENSOR_OK;

  Metadata meta = Metadata_init_zero;
  meta.ts = 1111;
  meta.logger_id = 2222;
  meta.cell_id = 3333;
  
  int64_t value = -1234567890123456789LL;

  uint8_t buffer[256];
  size_t buffer_len = sizeof(buffer);
  status = EncodeInt32Measurement(meta, value, SensorType_POWER_VOLTAGE, 
                              buffer, &buffer_len);

  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_GREATER_THAN(0, buffer_len);
}



void TestEncodeDoubleMeasurement(void) {
  SensorStatus status = SENSOR_OK;

  Metadata meta = Metadata_init_zero;
  meta.ts = 1111;
  meta.logger_id = 2222;
  meta.cell_id = 3333;

  double value = 12345.6789;

  uint8_t buffer[256];
  size_t buffer_len = sizeof(buffer);
  status = EncodeDoubleMeasurement(meta, value, SensorType_POWER_VOLTAGE, 
                              buffer, &buffer_len);

  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_GREATER_THAN(0, buffer_len);
}


void TestRepeatedSensorResponses(void) {
  SensorStatus status = SENSOR_OK;

  // prepare responses
  RepeatedSensorResponses responses = RepeatedSensorResponses_init_zero;
  responses.responses_count = 2;

  responses.responses[0].idx = 1;
  responses.responses[0].error = SensorError_OK;

  responses.responses[1].idx = 2;
  responses.responses[1].error = SensorError_DECODE;

  // encode
  uint8_t buffer[256];
  size_t buffer_len = sizeof(buffer);

  status = EncodeRepeatedSensorResponses(responses, responses.responses_count,
      buffer, &buffer_len);
  
  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_GREATER_THAN(0, buffer_len);

  // decode
  RepeatedSensorResponses decoded = RepeatedSensorResponses_init_zero;

  status = DecodeRepeatedSensorReponses(buffer, buffer_len, &decoded);
  
  TEST_ASSERT_EQUAL(SENSOR_OK, status);
  TEST_ASSERT_EQUAL(2, decoded.responses_count);

  TEST_ASSERT_EQUAL(responses.responses[0].idx, decoded.responses[0].idx);
  TEST_ASSERT_EQUAL(responses.responses[0].error, decoded.responses[0].error);

  TEST_ASSERT_EQUAL(responses.responses[1].idx, decoded.responses[1].idx);
  TEST_ASSERT_EQUAL(responses.responses[1].error, decoded.responses[1].error);
}


void TestCheckSensorResponse(void) {
  SensorStatus status = SENSOR_OK;
  SensorResponse resp = SensorResponse_init_zero;
    
  // Top level error (success)

  resp.idx = 0;
  resp.error = SensorError_OK;
  status = CheckSensorResponse(&resp);
  TEST_ASSERT_EQUAL(SENSOR_OK, status);

  // Top level error (failure)
  
  resp.idx = 0;
  resp.error = SensorError_GENERAL;
  status = CheckSensorResponse(&resp);
  TEST_ASSERT_EQUAL(SENSOR_REUPLOAD, status);


  // Individual measurement (success)
  
  resp.idx = 1;
  resp.error = SensorError_OK;
  status = CheckSensorResponse(&resp);
  TEST_ASSERT_EQUAL(SENSOR_OK, status);

  // Individual measurement (format)
  resp.idx = 2;
  resp.error = SensorError_LOGGER;
  status = CheckSensorResponse(&resp);
  TEST_ASSERT_EQUAL(SENSOR_FORMAT, status);


  // Individual measurement (reupload)
  resp.idx = 3;
  resp.error = SensorError_INVALID;
  status = CheckSensorResponse(&resp);
  TEST_ASSERT_EQUAL(SENSOR_REUPLOAD, status);
}


void TestRepeatedSensorMeasurementsSize(void) {
    SensorStatus status = SENSOR_OK;
    
    Metadata meta = Metadata_init_zero;
    meta.ts = 1000;
    meta.logger_id = 2000;
    meta.cell_id = 3000;
    
    SensorMeasurement in_array[5] = {};
    size_t len = 5;
    
    for (int i = 0; i < len; i++) {
        SensorMeasurement* in = &in_array[i];
    
        in->has_meta = true;
        in->meta.ts = 123 + i;
        in->meta.logger_id = 456 + i;
        in->meta.cell_id = 789 + i;
        in->type = SensorType_NONE;
    
        in->which_value = SensorMeasurement_unsigned_int_tag;
        in->value.unsigned_int = 98765430ULL + i;
    }
    
    size_t size = 0;
    status = RepeatedSensorMeasurementsSize(meta, in_array, len, &size);
    TEST_ASSERT_EQUAL(SENSOR_OK, status);
    TEST_ASSERT_GREATER_THAN(0, size);
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

  RUN_TEST(TestTranscodeSensorMeasurement);
  RUN_TEST(TestTranscodeRepeatedSensorMeasurements);
  RUN_TEST(TestEncodeUint32Measurement);
  RUN_TEST(TestEncodeInt32Measurement);
  RUN_TEST(TestEncodeDoubleMeasurement);
  RUN_TEST(TestRepeatedSensorResponses);
  RUN_TEST(TestCheckSensorResponse);
  RUN_TEST(TestRepeatedSensorMeasurementsSize);

  UNITY_END();
}
