/**
 * @file test_fifo.c
 * @brief Tests the fifo circular buffer library
 */

#include <stdio.h>
#include <unity.h>

#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "fifo.h"


void setUp(void) {
  FramBufferClear();
}

void tearDown(void) {}

void test_FramPut_ValidData(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};

  FramStatus status = FramPut(data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL(1, FramBufferLen());
}

void test_FramPut_BufferFull(void) {
  // Data size is larger than the buffer size
  uint8_t data[fram_buffer_size + 1];

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
  
  const int niters = fram_buffer_size / (sizeof(data)+1);

  // write 100 times, therefore 1100 bytes (data + len)
  for (int i = 0; i < niters; i++) {
    FramStatus status_zero = FramPut(data, sizeof(data));
    TEST_ASSERT_EQUAL(FRAM_OK, status_zero);
  }

  FramStatus status_full = FramPut(data, sizeof(data));
  TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status_full);
}

void test_FramGet_BufferEmpty(void) {
  uint8_t data[fram_buffer_size];
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

  const int niters = (fram_buffer_size / (sizeof(data)+1));

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
  const uint16_t half_num_bytes = fram_buffer_size / 2;

  const uint8_t zeros[fram_buffer_size];
  
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

  for (int i = 0; i < (fram_buffer_size / block_size); i++) {
    // wrap round write head to just behind read
    // -1 is to account for the length byte
    status = FramPut(zeros, block_size);
    TEST_ASSERT_EQUAL(FRAM_OK, status);
  }

  status = FramPut(zeros, block_size);
  TEST_ASSERT_EQUAL(FRAM_BUFFER_FULL, status);
}

void test_FramInitBuffer(void) {
  // First, put some data in the buffer and save the state
  uint8_t data[] = {1, 2, 3, 4, 5, 6};
  FramPut(data, sizeof(data));
  FramSaveBufferState();
  
  // Clear the buffer (this simulates a reset)
  FramBufferClear();
  
  // Now initialize the buffer, which should restore the saved state
  FramStatus status = FramInitBuffer();
  
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL(1, FramBufferLen());  // Expect 1 item in buffer, not 6
}

void test_FramSaveBufferState(void) {
  // Clear the buffer to start fresh
  FramBufferClear();
  
  // Put some data in the buffer
  uint8_t data[] = {1, 2, 3, 4, 5};
  FramStatus status = FramPut(data, sizeof(data));
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  
  // Save the buffer state
  status = FramSaveBufferState();
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  
  // Clear the buffer (this simulates a reset)
  FramBufferClear();
  
  // Now initialize the buffer, which should restore the saved state
  status = FramInitBuffer();
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  
  // Check if the buffer length is restored correctly
  TEST_ASSERT_EQUAL(1, FramBufferLen());
}

void test_FramGetReadAddr(void) {
  uint16_t read_addr = FramGetReadAddr();
  TEST_ASSERT_EQUAL(FRAM_BUFFER_START, read_addr);
}

void test_FramGetWriteAddr(void) {
  uint16_t write_addr = FramGetWriteAddr();
  TEST_ASSERT_EQUAL(FRAM_BUFFER_START, write_addr);
}

void test_FramGetBufferLen(void) {
  uint16_t buffer_len = FramGetBufferLen();
  TEST_ASSERT_EQUAL(0, buffer_len);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
  // test data recovery
  RUN_TEST(test_FramInitBuffer);
  RUN_TEST(test_FramSaveBufferState);
  RUN_TEST(test_FramGetReadAddr);
  RUN_TEST(test_FramGetWriteAddr);
  RUN_TEST(test_FramGetBufferLen);
  UNITY_END();
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3|RCC_CLOCKTYPE_HCLK
                              |RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_1);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
