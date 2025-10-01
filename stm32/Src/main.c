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

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// peripherials
#include "app_lorawan.h"

// userland
#include "ads.h"
#include "adc.h"
#include "bme280_sensor.h"
#include "board.h"
#include "controller/controller.h"
#include "controller/wifi.h"
#include "controller/wifi_userconfig.h"
#include "phytos31.h"
#include "sensors.h"
#include "status_led.h"
#include "teros12.h"
#include "teros21.h"
#include "userConfig.h"
#include "wifi.h"
#include "waterPressure.h"
#include "sen0308.h"
#include "waterFlow.h"
#include "user_config.h"

// Board configuration - define ONLY ONE of these
// Comment these out to disable sensors
#define DEFAULT
//#define USE_CAP_SOIL_SENSOR
//#define USE_WATER_PRESSURE_SENSOR
//#define USE_FLOW_METER_SENSOR


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
  MX_DMA_Init();
  MX_ADC_Init();
  MX_USART1_UART_Init();
  MX_I2C2_Init(); 
  SystemApp_Init(); 

  APP_PRINTF("\n\nRESET!\n\n");
  const char header[] = R"""(
+-----------------------------------+
|   ______  _   _  _______  _____   |
|  |  ____|| \ | ||__   __|/ ____|  |
|  | |__   |  \| |   | |  | (___    |
|  |  __|  | . ` |   | |   \___ \   |
|  | |____ | |\  |   | |   ____) |  |
|  |______||_| \_|   |_|  |_____/   |
|                                   |
|  Environmentally NeTworked Sensor |
+-----------------------------------+
)"""; 
  APP_PRINTF("\n%s\n", header);
  APP_PRINTF("Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n", __DATE__, __TIME__);
  APP_PRINTF("Git SHA: %s\n\n", GIT_REV);

  // Start status LEDs
  StatusLedInit();
  StatusLedFlashSlow();
  
  // initialize esp32 controller module  
  ControllerInit(); 
  
  // Print warning when using TEST_USER_CONFIG
#ifdef TEST_USER_CONFIG
  APP_LOG(TS_OFF, VLEVEL_M, "WARNING: TEST_USER_CONFIG is enabled!\n");
#endif  // TEST_USER_CONFIG

  UserConfigStart(60);
  const UserConfiguration* cfg = UserConfigGet();

  // initialize the user config interrupt
  //UserConfig_InitAdvanceTrace();
  
  // currently not functional
  //FIFO_Init();
  
  APP_LOG(TS_OFF, VLEVEL_M, "Enabling Sensors\n");
  APP_LOG(TS_OFF, VLEVEL_M, "----------------\n");

  // init senors interface
  SensorsInit();


  // configure enabled sensors
  for (int i = 0; i < cfg->enabled_sensors_count; i++) {
    EnabledSensor sensor = cfg->enabled_sensors[i];
    if ((sensor == EnabledSensor_Voltage) || (sensor == EnabledSensor_Current)) {
      #ifdef DEFAULT
      ADC_init();
      SensorsAdd(ADC_measure);
      APP_LOG(TS_OFF, VLEVEL_M, "ADC Enabled!\n");
      #endif

      #ifdef USE_FLOW_METER_SENSOR
      FlowInit();
      SensorsAdd(WatFlow_measure);
      APP_LOG(TS_OFF, VLEVEL_M, "Flow Meter Enabled!\n");
      #endif

      #ifdef USE_WATER_PRESSURE_SENSOR
      PressureInit();
      SensorsAdd(WatPress_measure);
      APP_LOG(TS_OFF, VLEVEL_M, "Water Pressure Sensor Enabled!\n");
      #endif

      #ifdef USE_CAP_SOIL_SENSOR
      CapSoilInit();
      SensorsAdd(SEN0308_measure);
      APP_LOG(TS_OFF, VLEVEL_M, "Cap Soil Sensor Enabled!\n");
      #endif

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
    // TODO add phytos31 support
    // TODO add support for dummy sensor
  }

  StatusLedFlashFast();
  
  APP_LOG(TS_OFF, VLEVEL_M, "\n\n");

  // init either WiFi or LoRaWAN
  if (cfg->Upload_method == Uploadmethod_LoRa) {
    MX_LoRaWAN_Init();
  } else if (cfg->Upload_method == Uploadmethod_WiFi) {
    WiFiInit();
  } else {
    APP_LOG(TS_ON, VLEVEL_M, "Invalid upload method!\n");
    Error_Handler();
  }

#ifdef SAVE_TO_MICROSD
  ControllerMicroSDUserConfig(cfg, SAVE_TO_MICROSD_FILENAME);
#endif


  while (1) {
    MX_LoRaWAN_Process();

    #ifdef USE_FLOW_METER_SENSOR
    FlowBackgroundTask();
    #endif
  }
}

// TODO update this to use the scheduler
#ifdef USE_FLOW_METER_SENSOR
void FlowBackgroundTask(void) {
  static uint32_t last_check = 0;
  uint32_t current_time = HAL_GetTick();
  
  // Update flow measurement every 100ms
  if (current_time - last_check >= 100) {
    FlowGetMeasurment(); // This updates the internal state
    last_check = current_time;
  }
}
#endif
