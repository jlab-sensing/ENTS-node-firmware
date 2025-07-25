/**
 * @file main_helper.h
 * @author John Madden (jmadden173@pm.me)
 * @brief Helper functions for main
 * @version 0.1
 * @date 2024-09-24
 *
 * Includes the SystemClock configuration and error handling. This file is
 * supplemental to "main.h".
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef TEST_MAIN_HELPER_H_
#define TEST_MAIN_HELPER_H_

#include <stdint.h>

#include "main.h"

void SystemClock_Config(void);

void assert_failed(uint8_t *file, uint32_t line);

#endif  // TEST_MAIN_HELPER_H_
