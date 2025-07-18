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

#include "board.h"
#include "gpio.h"
#include "main.h"
#include "page.h"
#include "usart.h"

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
  WaitForSerial();

  // Unit testing
  UNITY_BEGIN();

  // Tests for timestamp
  RUN_TEST(test_PageInit);

  UNITY_END();
}
