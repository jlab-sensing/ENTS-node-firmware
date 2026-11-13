/**
 * @example example_modbus.c
 *
 * Reads holding registers from a Modbus RTU slave device over UART and
 * prints the values using application logging.
 *
 * @author Kai Otsuka
 * @date 2026-11-12
 */

// Includes     
#include <stdio.h>

#include "board.h"
#include "gpio.h"
#include "main.h"
#include "modbus.h"
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
  MX_LPUART1_UART_Init();
  MX_USART2_UART_Init();

  uint16_t registers[4] = {0};
  uint32_t read_count = 1;

  while (1) {
    // Read 4 holding registers starting at address 0 from slave ID 1
    ModbusStatus status = ModbusReadRegister(1, 0x0000, 4, registers);

    if (status == MODBUS_OK) {
      APP_LOG(TS_OFF, VLEVEL_M, "%lu: Reg0=%u, Reg1=%u, Reg2=%u, Reg3=%u\r\n",
              read_count, registers[0], registers[1], registers[2],
              registers[3]);
    } else {
      APP_LOG(TS_OFF, VLEVEL_M, "%lu: Modbus read failed with status %d\r\n",
              read_count, status);
    }

    read_count++;
    HAL_Delay(1000);
  }
}