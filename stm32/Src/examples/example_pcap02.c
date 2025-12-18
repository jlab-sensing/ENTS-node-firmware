/**
 * @example example_pcap02.c
 *
 * Prints the C1 / C0 (single floating) capacitance ratio from the PCAP02
 * capacitance to digital sensor. C0 is the reference capacitor connected across
 * PC0 and PC1 of the plugin module. C1 is the capacitance measured across the
 * PC2 and PC3 ports.
 *
 * @author Jack Lin
 * @date 2025-08-18
 */

// Includes
#include <stdio.h>

#include "board.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "pcap02.h"
#include "sys_app.h"
#include "usart.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  SystemApp_Init();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C2_Init();
  MX_USART1_UART_Init();

  // Pin A10 (STM32WLE5JC) is used as an interrupt.
  // This function must be called after MX_GPIO_Init()
  pcap02_gpio_init();

  // NOTE: MX_I2C2_Init() must be called prior to pcap02_init().
  pcap02_init();

  // 8. ‘h40 03 00 00 00; Read Res1, addresses 3, 4, 5. Res1 is expected to be
  //                      in the range of 2,000,000 or ’h2000XX if the two
  //                      capacitors are of same size. Res1 has the format of a
  //                      fixed point number with 3 integer digits and 21
  //                      fractional digits. So, dividing the 2,000,000 by 2^21
  //                      gives a factor of about 1 for the ratio C1/C0.

  // 2^21 == 2,097,152

  // Note: The PCAP02 measures capacitance as a ratio. Due to the 3.21 fixed
  // point formatting, the minimum and maximum ratios are 0 and (8 - 1 / 2^21).
  // i.e. [0, 8) or to be more precise [0, 7.999999523162841796875]

  // The default onboard capacitors on the evaluation plugin module are 47 pF.
  // The measureable capacitance at RES1 (PC2 PC3) is 0 pF to 8*47 pF = 376 pF.

  pcap02_result_t result = {0};

  uint32_t conv = 1;

  while (1) {
    // Send CDC_Start_Conversion opcode
    pcap02_start_conversion();
    // Wait for the conversion to resolve (< 4 ms?)
    while (pcap02_measure_capacitance(&result) != 0);
    // Evaluate the conversion
    // APP_LOG(TS_OFF, VLEVEL_M,
    //         "0x%02X%02X%02X (0x%06X)\r\n"
    //         "\tfixed  (3): %01d\r\n"
    //         "\tfract (21): %d / %d = %f\r\n"
    //         "\tC1/C0     : %f\r\n"
    //         "\tC1        : %f pF\r\n",
    //         result.byte[2], result.byte[1], result.byte[0], result.word,
    //         result.fixed, result.fractional, (1 << 21),
    //         result.fractional / ((float)(1 << 21)), fixed_to_double(&result),
    //         PCAP02_REFERENCE_CAPACITOR_PF * fixed_to_double(&result));
    APP_LOG(TS_OFF, VLEVEL_M, "%d: %lf\r\n", conv, fixed_to_double(&result));
    conv++;
    HAL_Delay(100);
  }
}
