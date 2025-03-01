/**
 * @file test_main.cpp
 * @author John Madden (jmadden173@pm.me)
 * @brief Template file for tests
 * @version 0.1
 * @date 2024-08-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include <unity.h>

void setUp(void) {}

void tearDown(void) {}

void test_true(void) { TEST_ASSERT_TRUE(true); }

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(test_true);

  UNITY_END();
}

void loop() {}
