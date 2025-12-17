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

// static HAL_StatusTypeDef ret = HAL_OK;
// static volatile uint16_t dev_addr = PCAP02_I2C_ADDRESS;

// static volatile pcap02_result_t RES1;

// volatile uint32_t My_INTN_Counter = 0;
// volatile uint8_t My_INTN_State = 1;

/* Private function prototypes -----------------------------------------------*/
// float fixed_to_float(pcap02_result_t *RESx);

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
  pcap02_gpio_init();

  // NOTE: MX_I2C2_Init() must be called prior to pcap02_init().
  pcap02_init();

  /*
  APP_LOG(TS_OFF, VLEVEL_M,"Sweep I2C addresses.\r\n");
  uint8_t slave[255] = {0};
  I2C_Sweep_DevAddr(0, 127, slave);
  for (uint8_t i = 0; i < sizeof(slave); i++) {
    if (slave[i] == 0) continue;
    APP_LOG(TS_OFF, VLEVEL_M,"%d 0x%02X\r\n", i, slave[i]);
  }
  */

  // 8. ‘h40 03 00 00 00; Read Res1, addresses 3, 4, 5. Res1 is expected to be
  //                      in the range of 2,000,000 or ’h2000XX if the two
  //                      capacitors are of same size. Res1 has the format of a
  //                      fixed point number with 3 integer digits and 21
  //                      fractional digits. So, dividing the 2,000,000 by 2^21
  //                      gives a factor of about 1 for the ratio C1/C0.

  // 2^21 == 2,097,152

  // char print_buffer[256];
  pcap02_result_t result = {0};
  uint32_t previous_tick, current_tick;
  previous_tick = HAL_GetTick();

  while (1) {
    if (pcap02_measure_capacitance(&result) != 0) {
      continue;
    }

    current_tick = HAL_GetTick();
    if ((current_tick - previous_tick) > 1000) {
      // APP_LOG unable to print float?
      // Workaround: allocate own buffer and print string
      // APP_LOG (sys_app.h)
      //    --> UTIL_ADV_TRACE_COND_FSend (stm32_adv_trace.c)
      //    --> UTIL_ADV_TRACE_VSNPRINTF (utilities_conf.h)
      //    --> tiny_vsnprintf_like (stm32_tiny_vsnprintf.h)
      //        ^ This function does not support %f
      // snprintf(print_buffer, sizeof(print_buffer), );
      APP_LOG(TS_OFF, VLEVEL_M,
              "0x%02X%02X%02X (0x%06X)\r\n"
              "\tfixed  (3): %01d\r\n"
              "\tfract (21): %d / %d = %lf\r\n"
              "\tC1/C0     : %lf\r\n",
              result.byte[2], result.byte[1], result.byte[0], result.word,
              result.fixed, result.fractional, (1 << 21),
              result.fractional / ((double)(1 << 21)),
              result.fixed + result.fractional / ((double)(1 << 21)));
      previous_tick = current_tick;
    }
  }
}

// void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
//   /* Prevent unused argument(s) compilation warning */
//   UNUSED(GPIO_Pin);

//   // Note: It takes about 1us after INTN

//   if (GPIO_Pin == PCAP02_INTN_Pin) {
//     My_INTN_State = (HAL_GPIO_ReadPin(PCAP02_INTN_GPIO_Port, PCAP02_INTN_Pin)
//     ==
//                      GPIO_PIN_SET); /* low active */
//     if (My_INTN_State == 0) {
//       My_INTN_Counter += 1;
//     }
//   }
// }

// float fixed_to_float(pcap02_result_t *RESx) {
//   return ((float)RESx->fixed) +
//          ((float)(RESx->fractional) /
//           (1 << PCAP02_STANDARD_FIRMWARE_RESULT_FRACTIONAL_BITS));
// }
