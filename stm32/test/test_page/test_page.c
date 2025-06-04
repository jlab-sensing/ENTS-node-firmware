/**
 * @file test_page.c
 * @brief Test from non-volatile memory paging to sd card
 *
 * This test requires the esp32 to have the correct firmware loaded.
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2024-08-06
 */

#include <stdio.h>
#include <unity.h>

#include "gpio.h"
#include "main.h"
#include "page.h"
#include "usart.h"

void SystemClock_Config(void);

/**
 * @brief Setup code that runs at the start of every test
 *
 * Initialises library to know state
 */
void setUp(void) { PageInit(); }

/**
 * @brief Tear down code that runs at the end of every test
 *
 * Clean up memory from linked list
 */
void tearDown(void) { PageDeinit(); }

void test_PageInit(void) {
  // check front
  Page *front = PageFront();
  TEST_ASSERT_EQUAL(NULL, back);

  // check back
  Page *back = PageBack();
  TEST_ASSERT_EQUAL(NULL, back);

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(0, size);

  // check empty
  bool empty = PageEmpty();
  TEST_ASSERT_EQUAL(true, empty);
}

void test_PageEmpty(void) {
  bool empty = PageEmpty();
  TEST_ASSERT_TRUE(empty);
}

void test_PageSize(void) {
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(0, size);

  PagePushFront();
  size = PageSize();
  TEST_ASSERT_EQUAL(1, size);

  PagePushFront();
  size = PageSize();
  TEST_ASSERT_EQUAL(2, size);

  PagePushFront();
  size = PageSize();
  TEST_ASSERT_EQUAL(3, size);
}

void test_PagePushFront_single(void) {
  // push page to front
  Page *page = PagePushFront();

  // get front and  back
  Page *front = PageFront();
  Page *back = PageBack();

  // check for NULL
  TEST_ASSERT_NOT_NULL(page);

  // check front and back are equal
  TEST_ASSERT_EQUAL(page, front);
  TEST_ASSERT_EQUAL(page, back);
  TEST_ASSERT_EQUAL(NULL, page->next);
  TEST_ASSERT_EQUAL(NULL, page->prev);
  TEST_ASSERT_FALSE(page->open);

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(1, size);
}

void test_PagePushFront_multiple(void) {
  // add 3 elements
  for (int i = 0; i < 3; i++) {
    PagePushFront();
  }

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(3, size);

  // check first element
  Page *page = PageFront();
  TEST_ASSERT_NOT_NULL(page);
  TEST_ASSERT_NULL(page->prev);
  TEST_ASSERT_NOT_NULL(page->next);
  TEST_ASSERT_EQUAL(false, page->open);
  TEST_ASSERT_EQUAL(0, file_counter);

  // check second element
  Page *page_next = page->next;
  TEST_ASSERT_EQUAL(page, page_next->prev)
  TEST_ASSERT_NOT_NULL(page_next->next);
  TEST_ASSERT_EQUAL(false, page_next->open);
  TEST_ASSERT_EQUAL(1, file_counter);

  // check third element
  page = page_next;
  page_next = page->next;
  TEST_ASSERT_EQUAL(page, page_next->prev)
  TEST_ASSERT_NULL(page_next->next);
  TEST_ASSERT_EQUAL(false, page_next->open);
  TEST_ASSERT_EQUAL(2, file_counter);
}

void test_PagePushBack_single(void) {
  // push page to back
  Page *page = PagePushBack();

  // get front and  back
  Page *front = PageFront();
  Page *back = PageBack();

  // check for NULL
  TEST_ASSERT_NOT_NULL(page);

  // check front and back are equal
  TEST_ASSERT_EQUAL(page, front);
  TEST_ASSERT_EQUAL(page, back);
  TEST_ASSERT_EQUAL(NULL, page->next);
  TEST_ASSERT_EQUAL(NULL, page->prev);
  TEST_ASSERT_FALSE(page->open);

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(size, 1);
}

void test_PagePushBack_multiple(void) {
  // add 3 elements
  for (int i = 0; i < 3; i++) {
    PagePushBack();
  }

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(3, size);

  // check first element
  Page *page = PageFront();
  TEST_ASSERT_NOT_NULL(page);
  TEST_ASSERT_NULL(page->prev);
  TEST_ASSERT_NOT_NULL(page->next);
  TEST_ASSERT_EQUAL(false, page->open);
  TEST_ASSERT_EQUAL(0, file_counter);

  // check second element
  Page *page_next = page->next;
  TEST_ASSERT_EQUAL(page, page_next->prev)
  TEST_ASSERT_NOT_NULL(page_next->next);
  TEST_ASSERT_EQUAL(false, page_next->open);
  TEST_ASSERT_EQUAL(1, file_counter);

  // check third element
  page = page_next;
  page_next = page->next;
  TEST_ASSERT_EQUAL(page, page_next->prev)
  TEST_ASSERT_NULL(page_next->next);
  TEST_ASSERT_EQUAL(false, page_next->open);
  TEST_ASSERT_EQUAL(2, file_counter);
}

void test_PagePopFront_single(void) {
  PagePushFront();

  size_t size = PageSize();
  TEST_ASSERT_EQUAL(1, size);

  PagePopFront();

  Page *front = PageFront();
  TEST_ASSERT_NULL(front);
  Page *back = PageBack();
  TEST_ASSERT_NULL(back);
  size = PageSize();
  TEST_ASSERT_EQUAL(0, size);
}

void test_PagePopFront_multiple(void) {
  // add 3 items to list
  for (int i = 0; i < 3; i++) {
    PagePushFront();
  }

  Page *front_pop = PageFront();

  PagePopFront();

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(2, size);

  // check pointers for first element
  Page *front = PageFront() TEST_ASSERT_NOT_EQUAL(front_pop, front);
  TEST_ASSERT_NULL(front->prev);
  TEST_ASSERT_NOT_NULL(front->next);
}

void test_pagePopBack_single(void) {
  PagePushFront();

  size_t size = PageSize();
  TEST_ASSERT_EQUAL(1, size);

  PagePopBack();

  Page *front = PageFront();
  TEST_ASSERT_NULL(front);
  Page *back = PageBack();
  TEST_ASSERT_NULL(back);
  size = PageSize();
  TEST_ASSERT_EQUAL(0, size);
}

void test_PagePopBack_multiple(void) {
  // add 3 items to list
  for (int i = 0; i < 3; i++) {
    PagePushFront();
  }

  Page *back_pop = PageBack();

  PagePopBack();

  // check size
  size_t size = PageSize();
  TEST_ASSERT_EQUAL(2, size);

  // check pointers for last element
  Page *back = PageBack() TEST_ASSERT_NOT_EQUAL(back_pop, back);
  TEST_ASSERT_NULL(back->next);
  TEST_ASSERT_NOT_NULL(back->prev);
}

void test_PageOpen(void) {
  Page *page = PagePushFront();

  TEST_ASSERT_EQUAL(false, page);

  PageOpen(page);

  TEST_ASSERT_EQUAL(true, page);
}

void test_PageClose(void) {
  Page *page = PagePushFront();

  PageOpen(page);

  TEST_ASSERT_EQUAL(true, page->open);

  PageClose(page);

  TEST_ASSERT_EQUAL(false, page->open);
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

  // Tests for timestamp
  RUN_TEST(test_PageInit);

  UNITY_END();
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK3 | RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK3Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
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
void Error_Handler(void) {
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /* USER CODE BEGIN Error_Handler_Debug */
  char error[30];
  int error_len =
      snprintf(error, sizeof(error), "Error!  HAL Status: %d\n", rc);
  HAL_UART_Transmit(&huart1, error, error_len, 1000);

  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
