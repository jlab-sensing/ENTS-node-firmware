/**
 * @example example_retrieve_data.c
 *
 * Entry point for managing and retrieving data stored in FRAM across
 * restarts.
 *
 * This code performs the following functions:
 * 1. Initializes the STM32 system, GPIO, I2C, UART, and other
 application-specific modules.
 * 2. Writes example data multiple times to a FRAM buffer to test data storage
 functionality.
 * 3. Blinks an LED on GPIO_PIN_5 of port GPIOB to indicate the program is
 running.
 * 4. (Optional) Includes a section to read and print stored data from FRAM,
 currently commented out.
 *
 * Expected behavior: The LED should blink continuously, and pressing the
 "Restart" button will
 * trigger additional data entries in the FRAM buffer.
 *
 * @author Jack Lin
 * @date NA
 */

#include <stdio.h>

#include "adc.h"
#include "battery.h"
#include "board.h"
#include "dma.h"
#include "fifo.h"
#include "fram.h"
#include "gpio.h"
#include "main.h"
#include "sys_app.h"
#include "usart.h"

/** Delay between print statements */
#ifndef DELAY
#define DELAY 1000
#endif

HAL_StatusTypeDef rc;

int main(void) {
  HAL_Init();
  SystemClock_Config();
  SystemApp_Init();

  MX_GPIO_Init();
  MX_USART1_UART_Init();
  MX_DMA_Init();
  MX_I2C2_Init();

  FIFO_Init();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // Set as push-pull output
  GPIO_InitStruct.Pull = GPIO_NOPULL;           // No pull-up or pull-down
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // Low frequency for LED

  FramStatus status;

  const uint8_t test_data[] = {0x11, 0x22, 0x33};  // Example data
  for (int i = 0; i < 10; i++) {
    status = FramPut(test_data, sizeof(test_data));
  }

  uint16_t read_addr, write_addr, buffer_len;
  status = FramLoadBufferState(&read_addr, &write_addr, &buffer_len);

  uint8_t retrieved_data[sizeof(test_data)];
  uint8_t retrieved_len;

  APP_PRINTF("Press the Restart button to add more data to FRAM...\n");

  // Visualizing actual data (commented out)
  // while (FramBufferLen() > 0) {
  //     status = FramGet(retrieved_data, &retrieved_len);
  //     if (status == FRAM_OK) {
  //         // Loop through retrieved_data and print each byte in hexadecimal
  //         format print("Data length: %d\n", retrieved_len); for (int i = 0; i
  //         < retrieved_len; i++) {
  //             APP_PRINTF("Data[%d]: 0x%02X\n", i, retrieved_data[i]);
  //         }
  //     } else {
  //         // Handle error if needed
  //         break; // Exit the loop if an error occurs
  //     }
  // }

  while (1) {
    char buf[32];
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_5);
    HAL_Delay(500);
  }
}
