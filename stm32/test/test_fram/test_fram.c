/**
 * @file test_fram.c
 * @brief Tests the fram library
 */

#include <stdio.h>
#include <unity.h>

#include "main.h"
#include "i2c.h"
#include "usart.h"
#include "gpio.h"
#include "fram.h"

void setUp(void) {}

/**
 * @brief Tear down code that runs at the end of every test
*/
void tearDown(void) {}

void test_i2c(void){
  uint8_t test = 185;
  uint8_t received;

  HAL_StatusTypeDef status;

  // test raw i2c write
  status = HAL_I2C_Mem_Write(&hi2c2, 0xA0, 0x00, I2C_MEMADD_SIZE_8BIT, &test,
                             1, 50);
  TEST_ASSERT_EQUAL(HAL_OK, status);

  // test raw i2c read
  status = HAL_I2C_Mem_Read(&hi2c2, 0xA1, 0x00, I2C_MEMADD_SIZE_8BIT, &received,
                            1, 50);
  TEST_ASSERT_EQUAL(HAL_OK, status);
  TEST_ASSERT_EQUAL(test, received);
}

void test_FramWrite_ValidData(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  uint16_t addr = 0x00;

  FramStatus status = FramWrite(addr, data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramWrite_OutOfRange(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  uint16_t addr = fram_max_addr + 1;  // Address out of range

  FramStatus status = FramWrite(addr, data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OUT_OF_RANGE, status);
}

void test_FramWrite_ZeroLength(void) {
  uint8_t data[] = {1, 2, 3, 4, 5};
  uint16_t addr = 0x00;

  FramStatus status = FramWrite(addr, data, 0);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramWrite_MultiplePages(void) {
  uint8_t data [] = {1, 2, 3, 4, 5};
  // right below the segment size
  uint16_t addr = FRAM_SEG_SIZE-1;

  FramStatus status = FramWrite(addr, data, sizeof(data));

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramRead_Status(void) {
  uint8_t data[5];
  uint16_t addr = 0x0;

  FramStatus status = FramRead(addr, sizeof(data), data);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramRead_ValidData(void) {
  uint16_t addr = 0x00;

  uint8_t write_data[] = {1, 2, 3, 4, 5};
  FramWrite(addr, write_data, sizeof(write_data));

  uint8_t read_data[5];
  FramStatus status = FramRead(addr, sizeof(read_data), read_data);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(write_data, read_data, 5);
}

void test_FramRead_ZeroLength(void) {
  uint8_t data[5];
  uint16_t addr = 0x0;

  FramStatus status = FramRead(addr, 0, data);

  TEST_ASSERT_EQUAL(FRAM_OK, status);
}

void test_FramRead_OutOfRange(void) {
  uint8_t data[5];
  uint16_t addr = fram_max_addr + 1;

  FramStatus status = FramRead(addr, sizeof(data), data);

  TEST_ASSERT_EQUAL(FRAM_OUT_OF_RANGE, status);
}

void test_FramRead_MultiplePages(void) {
  // right below the segment size
  uint16_t addr = FRAM_SEG_SIZE-1;

  uint8_t write_data[] = {1, 2, 3, 4, 5}; 
  FramWrite(addr, write_data, sizeof(write_data));

  uint8_t read_data[5];
  FramStatus status = FramRead(addr, sizeof(read_data), read_data);  

  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(write_data, read_data, 5);
}

void test_FramSaveBufferState(void) {
  uint16_t read_addr = 100;
  uint16_t write_addr = 200;
  uint16_t buffer_len = 50;

  FramStatus status = FramSaveBufferState_Internal(read_addr, write_addr, buffer_len);
  TEST_ASSERT_EQUAL(FRAM_OK, status);

  uint16_t loaded_read_addr, loaded_write_addr, loaded_buffer_len;
  status = FramLoadBufferState(&loaded_read_addr, &loaded_write_addr, &loaded_buffer_len);
  
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL(read_addr, loaded_read_addr);
  TEST_ASSERT_EQUAL(write_addr, loaded_write_addr);
  TEST_ASSERT_EQUAL(buffer_len, loaded_buffer_len);
}

void test_FramLoadBufferState(void) {
  uint16_t read_addr = 150;
  uint16_t write_addr = 250;
  uint16_t buffer_len = 75;

  FramSaveBufferState_Internal(read_addr, write_addr, buffer_len);

  uint16_t loaded_read_addr, loaded_write_addr, loaded_buffer_len;
  FramStatus status = FramLoadBufferState(&loaded_read_addr, &loaded_write_addr, &loaded_buffer_len);
  
  TEST_ASSERT_EQUAL(FRAM_OK, status);
  TEST_ASSERT_EQUAL(read_addr, loaded_read_addr);
  TEST_ASSERT_EQUAL(write_addr, loaded_write_addr);
  TEST_ASSERT_EQUAL(buffer_len, loaded_buffer_len);
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  // wait for UART
  for (int i = 0; i < 1000000; i++) {
      __NOP();
  }

  UNITY_BEGIN();
  RUN_TEST(test_i2c);
  RUN_TEST(test_FramWrite_ValidData);
  RUN_TEST(test_FramWrite_ZeroLength);
  RUN_TEST(test_FramWrite_OutOfRange); 
  RUN_TEST(test_FramWrite_MultiplePages);
  RUN_TEST(test_FramRead_ValidData);
  RUN_TEST(test_FramRead_ZeroLength);
  RUN_TEST(test_FramRead_OutOfRange); 
  RUN_TEST(test_FramRead_MultiplePages);
  RUN_TEST(test_FramSaveBufferState);
  RUN_TEST(test_FramLoadBufferState);
  UNITY_END();
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
