/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "app_lorawan.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "board.h"

#include <stdio.h>

#include "sys_app.h"
#include <stdlib.h>
#include <stdbool.h>

#include "ads.h"
#include "phytos31.h"
#include "bme280_sensor.h"
#include "rtc.h"
#include "sensors.h"
#include "wifi.h"
#include "controller/controller.h"
#include "controller/wifi.h"
#include "userConfig.h"
#include "teros12.h"
#include "teros21.h"
#include "status_led.h"
<<<<<<< HEAD
#include "waterPressure.h"
#include "sen0308.h"
#include "waterFlow.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

void SystemClock_Config(void);
=======
>>>>>>> main

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init(); 
  SystemApp_Init();

  // Start status LEDs
  StatusLedInit();
  StatusLedFlashSlow();

  // Try loading user config
  if (UserConfigLoad() != USERCONFIG_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error loading user configuration!\n");
    APP_LOG(TS_OFF, VLEVEL_M, "Waiting for new configuration...\n");

    // TODO implement status code with LED

    // Wait for new configuration
    //UserConfig_ProcessDataPolling();
    UserConfig_InitAdvanceTrace();
    while (1);
  }

  // Print user config
  UserConfigPrint();

  // required for SDI-12
  MX_USART2_UART_Init();
  MX_TIM1_Init();

  // initialize the user config interrupt
  UserConfig_InitAdvanceTrace();
  
  // placeholder for UserConfig polling
  HAL_Delay(10000);

  // alternative blocking polling method
  //UserConfig_ProcessDataPolling();

  // currently not functional
  //FIFO_Init();

  // Debug message, gets printed after init code
  APP_PRINTF("Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n", __DATE__, __TIME__);
  APP_PRINTF("Git SHA: %s\n", GIT_REV);

  // initialize esp32 controller module  
  ControllerInit();

  // get the current user config
  const UserConfiguration* cfg = UserConfigGet();
  
  // init senors interface
  SensorsInit();

  // configure enabled sensors
  for (int i=0; i < cfg->enabled_sensors_count; i++) {
    EnabledSensor sensor = cfg->enabled_sensors[i];
    if ((sensor == EnabledSensor_Voltage) || (sensor == EnabledSensor_Current)) {
      //ADC_init();
      //change correct pins for water flow measurement
      //FlowInit(); 
      PressureInit();
      SensorsAdd(WatPress_measure); //water pressure measurement
      //SensorsAdd(ADC_measure); //power measurement
      //SensorsAdd(SEN0308_measure); //capacitive soil measurement
      //SensorsAdd(WatFlow_measure); //water flow meter measurement
      //APP_LOG(TS_OFF, VLEVEL_M, "ADS Enabled!\n");
      APP_LOG(TS_OFF, VLEVEL_M, "Flow Meter Enabled!\n");
    }
    if (sensor == EnabledSensor_Teros12) {
      APP_LOG(TS_OFF, VLEVEL_M, "Teros12 Enabled!\n");
      SensorsAdd(Teros12Measure);
    }
    if (sensor == EnabledSensor_BME280) {
      BME280Init();
      SensorsAdd(BME280Measure);
      APP_LOG(TS_OFF, VLEVEL_M, "BME280 Enabled!\n");
    }
    if (sensor == EnabledSensor_Teros21) {
      SensorsAdd(Teros21Measure);
      APP_LOG(TS_OFF, VLEVEL_M, "Teros21 Enabled!\n");
    }
    // TODO add support for dummy sensor
  }
 
  StatusLedFlashFast();

  // init either WiFi or LoRaWAN
  if (cfg->Upload_method == Uploadmethod_LoRa) {
    MX_LoRaWAN_Init();
  } else if (cfg->Upload_method == Uploadmethod_WiFi) {
    WiFiInit();
  } else {
    APP_LOG(TS_ON, VLEVEL_M, "Invalid upload method!\n");
    Error_Handler();
  } 
  
  while (1)
  {
    MX_LoRaWAN_Process();
  }
<<<<<<< HEAD
  /* USER CODE END 3 */
}
=======
}

>>>>>>> main
