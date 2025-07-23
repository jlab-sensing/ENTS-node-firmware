// Copyright 2023 UCSC
/**
 * @file test_fifo.c
 * @brief Tests the fifo circular buffer library
 */

#include <stdio.h>
#include <unity.h>

#include "board.h"
#include "fifo.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
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

  // Set the memory after the FIFO buffer to a unique character to check OOB
  // memory write
  uint8_t oob_before = 0;
  uint8_t oob_after = 0;
  const uint8_t oob_check = 0xFF;
  status = FramRead(FRAM_BUFFER_END + 1, 1,
                    &oob_before);  // to be restored afterwards
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  status = FramWrite(FRAM_BUFFER_END + 1, &oob_check, 1);
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  // write block size to handle length
  // block_size+1 must not be a factor of the FIFO's space
  uint8_t block_size = 70;
  while (kFramBufferSize % (block_size + 1) == 0) {
    block_size += 1;
  }

  // oob_check is reserved as a special character for determining
  // if data was written out of bounds
  TEST_ASSERT_NOT_EQUAL(block_size, oob_check);
  TEST_ASSERT_NOT_EQUAL(block_size + 1, oob_check);

  uint8_t junk_data[256];
  for (int i = 0; i < 256; i++) {
    junk_data[i] = i;
  }

  uint8_t buffer[256];
  uint8_t buffer_length;

  // move write to before the end of physical memory in FRAM
  // if the assigned FRAM memory is [0, 1769], then a block_size of 16 (+1
  // length byte) means that the 104th block starts at 1768 and must wrap
  // around. read 0, write 1768
  status = FRAM_OK;
  while (status != FRAM_BUFFER_FULL) {
    TEST_ASSERT_EQUAL(FRAM_OK, status);
    status = FramPut(junk_data, block_size);
  }

  // advance read all the way to the end to make room for the wraparound
  // read 1768, write 1768
  while (FramBufferLen() != 0) {
    status = FramGet(buffer, &buffer_length);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
  }

  // write one block for the wraparound
  // read 1768, write 1768 + 17 = 14
  // [{68} 69 70 0 1 2 3 4 5 6 7 8 9 10 11 12 13] 14
  status = FramPut(junk_data, block_size);
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  // observe the wraparound
  // read 14, write 14
  status = FramGet(buffer, &buffer_length);
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  // test that the data was successfully retrieved across the wraparound
  TEST_ASSERT_EQUAL_UINT8_ARRAY(buffer, junk_data, block_size);

  // test that no data was written out of bounds
  status = FramRead(FRAM_BUFFER_END + 1, 1, &oob_after);
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL(oob_after, oob_check);
  status = FramWrite(FRAM_BUFFER_END + 1, &oob_before, 1);  // restore
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  // status = FramPut(zeros, block_size);
  // TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status);
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
  WaitForSerial();

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
