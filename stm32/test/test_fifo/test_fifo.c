// Copyright 2023 UCSC
/**
 * @file test_fifo.c
 * @brief Tests the fifo circular buffer library
 */

#include <stdio.h>
#include <unity.h>

#include "fifo.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "main_helper.h"
#include "usart.h"

void setUp(void) { FramBufferClear(); }

void tearDown(void) {}

void test_FramPut_ValidData(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};

  FramStatus status = FramPut(data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL(1, FramBufferLen());
}

void test_FramPut_BufferFull(void) {
  // Data size is larger than the buffer size
  const uint16_t kFullBufferSize = kFramBufferSize + 1;
  uint8_t data[kFullBufferSize];

  FramStatus status = FramPut(data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status);
}

void test_FramPut_Sequential(void) {
  const int niters = 20;

  // starting values
  uint8_t data[10] = {0, 1, 2, 3, 4};

  // write 100 times, therefore 1100 bytes (data + len)
  for (int i = 0; i < niters; i++) {
    FramStatus status = FramPut(data, sizeof(data));
    TEST_ASSERT_EQUAL(FRAM_OK, status);

    // increment index of data
    for (int j = 0; j < sizeof(data); j++) {
      data[j]++;
    }
  }
}

void test_FramPut_Sequential_BufferFull(void) {
  // starting values
  uint8_t data[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

  const int niters = kFramBufferSize / (sizeof(data) + 1);

  // write 100 times, therefore 1100 bytes (data + len)
  for (int i = 0; i < niters; i++) {
    FramStatus status_zero = FramPut(data, sizeof(data));
    TEST_ASSERT_EQUAL(FRAM_OK, status_zero);
  }

  FramStatus status_full = FramPut(data, sizeof(data));
  TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status_full);
}

void test_FramGet_BufferEmpty(void) {
  uint8_t data[kFramBufferSize];
  uint8_t data_len;

  FramStatus status = FramGet(data, &data_len);

  TEST_ASSERT_EQUAL(FRAM_BUFFER_EMPTY, status);
}

void test_FramGet_ValidData(void) {
  uint8_t put_data[] = {1, 2, 3, 4, 5};
  FramPut(put_data, sizeof(put_data));

  uint8_t get_data[sizeof(put_data)];
  uint8_t get_data_len;
  FramStatus status = FramGet(get_data, &get_data_len);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(put_data, get_data, sizeof(put_data));
}

void test_FramGet_Sequential(void) {
  const int niters = 20;

  // starting values
  uint8_t put_data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  // write 100 times, therefore 1100 bytes (data+len)
  for (int i = 0; i < niters; i++) {
    FramPut(put_data, sizeof(put_data));

    // increment index of data
    for (int j = 0; j < sizeof(put_data); j++) {
      put_data[j]++;
    }
  }

  // reset put_data to starting values
  // was unsure about the behavior of memcpy
  for (int i = 0; i < sizeof(put_data); i++) {
    put_data[i] = i;
  }

  uint8_t get_data[10] = {0};

  // read back all the data
  for (int i = 0; i < niters; i++) {
    uint8_t get_data_len = 0;
    FramStatus status_get = FramGet(get_data, &get_data_len);

    TEST_ASSERT_EQUAL(FRAM_OK, status_get);
    TEST_ASSERT_EQUAL_INT(10, get_data_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(put_data, get_data, sizeof(put_data));

    // increment index of data
    for (int j = 0; j < sizeof(put_data); j++) {
      put_data[j]++;
    }
  }
}

void test_FramGet_Sequential_BufferFull(void) {
  FramStatus status = FRAM_OK;

  // starting values
  uint8_t data[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

  const int niters = (kFramBufferSize / (sizeof(data) + 1));

  // write 100 times, therefore 1100 bytes (data + len)
  for (int i = 0; i < niters; i++) {
    status = FramPut(data, sizeof(data));
    TEST_ASSERT_EQUAL(FRAM_OK, status);

    // increment data
    for (int j = 0; j < sizeof(data); j++) {
      data[j] += sizeof(data);
    }
  }

  // try writing data
  status = FramPut(data, sizeof(data));
  TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status);
  // reset data
  for (int i = 0; i < sizeof(data); i++) {
    data[i] = i;
  }

  uint8_t get_data[sizeof(data)] = {0};

  for (int i = 0; i < niters; i++) {
    uint8_t get_data_len = 0;
    status = FramGet(get_data, &get_data_len);

    TEST_ASSERT_EQUAL(FRAM_OK, status);
    TEST_ASSERT_EQUAL_INT(sizeof(data), get_data_len);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data, get_data, sizeof(data));

    // increment data
    for (int j = 0; j < sizeof(data); j++) {
      data[j] += sizeof(data);
    }
  }
}

void test_FramBufferLen(void) {
  // Assuming that the buffer length is initially 0
  TEST_ASSERT_EQUAL(0, FramBufferLen());

  // Put some data into the buffer
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramPut(data, sizeof(data));

  // Now the buffer length should be equal to the size of the data
  TEST_ASSERT_EQUAL(1, FramBufferLen());
}

void test_FramBufferClear(void) {
  // Put some data into the buffer
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramStatus status = FramPut(data, sizeof(data));
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  TEST_ASSERT_EQUAL(1, FramBufferLen());

  // Clear the buffer
  FramBufferClear();

  // Now the buffer length should be 0
  TEST_ASSERT_EQUAL(0, FramBufferLen());
}

void test_FramBuffer_Wraparound(void) {
  // checks for errors when read addr > write addr
  FramStatus status;

  // write block size to handle length
  const uint8_t block_size = 255;

  // number of bytes to get halfway on the address space
  const uint16_t half_num_bytes = kFramBufferSize / 2;

  const uint8_t zeros[kFramBufferSize];

  uint8_t buffer[sizeof(zeros)];

  // move write to roughly halfway point
  for (int i = 0; i < (half_num_bytes / block_size); i++) {
    status = FramPut(zeros, block_size);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
  }

  while (FramBufferLen() != 0) {
    // move read head
    uint8_t data_read_len;
    status = FramGet(buffer, &data_read_len);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
  }

  for (int i = 0; i < (kFramBufferSize / block_size); i++) {
    // wrap round write head to just behind read
    // -1 is to account for the length byte
    status = FramPut(zeros, block_size);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
  }

  status = FramPut(zeros, block_size);
  TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status);
}

void test_LoadSaveBufferState(void) {
  // Clear the buffer and check initial state
  FramStatus status = FramBufferClear();
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  // Add some data
  const uint8_t test_data[] = {0x11, 0x22, 0x33};
  for (int i = 0; i < 10; i++) {
    // note that FramPut() calls FramSaveBufferState()
    status = FramPut(test_data, sizeof(test_data));
    TEST_ASSERT_EQUAL(FRAM_OK, status);
  }

  // Load the new buffer state
  uint16_t saved_read_addr, saved_write_addr, saved_buffer_len;
  status = FramLoadBufferState(&saved_read_addr, &saved_write_addr,
                               &saved_buffer_len);
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  // Validate the updated state: buffer_len should reflect 10 puts
  TEST_ASSERT_EQUAL(10, saved_buffer_len);
  TEST_ASSERT_NOT_EQUAL(FRAM_BUFFER_START, saved_write_addr);
  TEST_ASSERT_EQUAL(FRAM_BUFFER_START, saved_read_addr);

  uint8_t retrieved_data[sizeof(test_data)];
  uint8_t retrieved_len;
  for (int i = 0; i < 10; i++) {
    status = FramGet(retrieved_data, &retrieved_len);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(test_data, retrieved_data, sizeof(test_data));
  }
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface
  and the Systick. */
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
  MX_I2C2_Init();
  /* USER CODE BEGIN 2 */

  // wait for UART
  for (int i = 0; i < 1000000; i++) {
    __NOP();
  }

  UNITY_BEGIN();
  // other tests depend on the following two
  RUN_TEST(test_FramBufferClear);
  RUN_TEST(test_FramBufferLen);
  // test buffer functions
  RUN_TEST(test_FramPut_ValidData);
  RUN_TEST(test_FramPut_BufferFull);
  RUN_TEST(test_FramPut_Sequential);
  RUN_TEST(test_FramPut_Sequential_BufferFull);
  RUN_TEST(test_FramGet_ValidData);
  RUN_TEST(test_FramGet_BufferEmpty);
  RUN_TEST(test_FramGet_Sequential);
  RUN_TEST(test_FramGet_Sequential_BufferFull);
  RUN_TEST(test_FramBuffer_Wraparound);
  RUN_TEST(test_LoadSaveBufferState);
  UNITY_END();
  /* USER CODE END 3 */
}
