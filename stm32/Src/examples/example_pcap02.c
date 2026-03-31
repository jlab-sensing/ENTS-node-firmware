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

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();
  SystemApp_Init();

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

  pcap02_result_t res1 = {0}, res2 = {0}, res3 = {0};

  uint32_t conv = 1;
  double res1_double, res2_double, res3_double;

  while (1) {
    pcap02_measure_capacitance(&res1, &res2, &res3);
    res1_double = fixed_to_double(&res1);
    res2_double = fixed_to_double(&res2);
    res3_double = fixed_to_double(&res3);
    APP_LOG(TS_OFF, VLEVEL_M,
            "%d (47pF): C1/C0 = %lf, %lf pF, C2/C0 = %lf, %lf pF, C3/C0 = %lf, "
            "%lf pF\r\n",
            conv, res1_double, res1_double * 47, res2_double, res2_double * 47,
            res3_double, res3_double * 47);
    conv++;
    HAL_Delay(1000);
  }
}
