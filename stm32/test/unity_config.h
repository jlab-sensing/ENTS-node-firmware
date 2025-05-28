/**
 * @brief Unity test framework configuration for STM32 HAL libraries
 *
 *
 *
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-27
 */

#ifndef TEST_UNITY_CONFIG_H_
#define TEST_UNITY_CONFIG_H_

#include "usart.h"

#ifndef NULL
#ifndef __cplusplus
#define NULL (void *)0
#else
#define NULL 0
#endif
#endif

/** Allow float comparisons */
#define UNITY_INCLUDE_FLOAT
/** Allow double comparisons*/
#define UNITY_INCLUDE_DOUBLE

/** Timeout for UART transmission */
#define UART_TIMEOUT 1000

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Transmits single charater to USART1
 *
 * The return code is checked. Requires the USART library to be initialized.
 *
 * @param c Character to transmit
 */
void UnityOutputChar(char c);

/** @see UnityOutputChar */
#define UNITY_OUTPUT_CHAR(c) UnityOutputChar(c);

#ifdef __cplusplus
}
#endif  // extern "C"

#endif  // TEST_UNITY_CONFIG_H_
