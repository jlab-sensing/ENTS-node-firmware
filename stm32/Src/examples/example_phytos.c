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
  MX_USART2_UART_Init();
  MX_I2C1_Init();

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
  HAL_UART_Transmit(&huart2, (const uint8_t *)info_str, info_len, 1000);

  UserConfiguration cfg = UserConfiguration_init_zero;
  cfg.enabled_sensors_multiple_count = 2;  // not used in this example

  cfg.enabled_sensors_multiple[0].cell_id = 200;  // not used in this example
  cfg.enabled_sensors_multiple[0].enabled_sensor = EnabledSensor_Current;
  cfg.enabled_sensors_multiple[0].index = 0x40 << 1;
  // cfg.enabled_sensors_multiple[1].cell_id = 200;  // not used in this example
  // cfg.enabled_sensors_multiple[1].enabled_sensor = EnabledSensor_Voltage;
  // cfg.enabled_sensors_multiple[1].index = 0x40 << 1;

  /* USER CODE BEGIN 2 */
  Phytos31Init(&(cfg.enabled_sensors_multiple[0]));
  // TIMER_IF_Init();
  // __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
  // UTIL_TIMER_Init();

  char output[20];
  phytos_measurements measurement;
  size_t reading_len;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    measurement = Phytos31GetMeasurement(&(cfg.enabled_sensors_multiple[0]));
    reading_len = snprintf(output, sizeof(output), "Phytos Raw: %f\r\n",
                           measurement.phytos31_raw);
    HAL_UART_Transmit(&huart2, (const uint8_t *)output, reading_len,
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