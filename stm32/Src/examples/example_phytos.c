/**
 * @example example_phytos.c
 *
 * Connect the PHYTOS-31 sensor to the analog input of the ents-node*. An
 * infinite loop will read the sensor and print the measurement over serial.
 *
 * @author Stephen Taylor
 * @date NA
 */

// stdlib includes
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// system includes
#include "adc.h"
#include "app_lorawan.h"
#include "board.h"
#include "dma.h"
#include "gpio.h"
#include "i2c.h"
#include "rtc.h"
#include "sys_app.h"
#include "usart.h"

// user includes
#include "ads.h"
#include "phytos31.h"
#include "sdi12.h"

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init();

  SystemApp_Init();

  UserConfigLoad();

  // TIMER_IF_Init();
  /* USER CODE BEGIN 2 */

  // Print the compilation time at startup
  char info_str[100];
  int info_len;
  info_len = snprintf(info_str, sizeof(info_str),
                      "Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n",
                      __DATE__, __TIME__);
  HAL_UART_Transmit(&huart1, (const uint8_t *)info_str, info_len, 1000);

  /* USER CODE BEGIN 2 */
  Phytos31Init();
  // TIMER_IF_Init();
  // __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  // UTIL_TIMER_Init();

  char output[20];
  char output2[20];

  double voltage_reading;
  phytos_measurments measurment;
  size_t reading_len;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    measurment = Phytos31GetMeasurment();
    reading_len = snprintf(output, sizeof(output), "Phytos Raw: %f\r\n",
                           measurment.phytos31_raw);
    HAL_UART_Transmit(&huart1, (const uint8_t *)output, reading_len,
                      HAL_MAX_DELAY);
    // for (int i = 0; i < 10000; i++){
    //   asm("nop");
    // }

    HAL_Delay(
        1000);  // I guess HAL_Delay is broken somehow, don't understand why
    // for (int i = 0; i < 1000000; i++){
    //   asm("nop");
    // }
  }
  /* USER CODE END 3 */
}
