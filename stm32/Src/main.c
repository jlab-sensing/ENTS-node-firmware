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
#include <string.h>

#include "ads.h"
#include "phytos31.h"
#include "bme280_sensor.h"
#include "rtc.h"
#include "sensors.h"
#include "wifi.h"
#include "controller/controller.h"
#include "controller/wifi.h"
#include "controller/wifi_userconfig.h"
#include "userConfig.h"
#include "teros12.h"
#include "teros21.h"
#include "status_led.h"

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
  
  // initialize esp32 controller module  
  ControllerInit(); 

  // variables to store WiFi host info
  char ssid[255] = {};
  char ip[16] = {};
  char mac[18] = {};
 
  // constant password for AP
  const char pass[] = "ilovedirt";

  // Print warning when using TEST_USER_CONFIG
#ifdef TEST_USER_CONFIG
  APP_LOG(TS_OFF, VLEVEL_M, "WARNING: TEST_USER_CONFIG is enabled!\n");
#endif  // TEST_USER_CONFIG
  
  // Try loading user config
  // If we fail, assume that it is empty and start user config website to get
  // new configuration
  if (UserConfigLoad() != USERCONFIG_OK) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error loading user configuration!\n");
    APP_LOG(TS_OFF, VLEVEL_M, "Waiting for new configuration...\n");

    // start user config interface
    const char ssid_unconfigured[] = "ents-unconfigured";
    ControllerWiFiHost(ssid_unconfigured, pass);

    ControllerWiFiHostInfo(ssid, ip, mac);
    APP_LOG(TS_OFF, VLEVEL_M, "ssid \"%s\"\n", ssid);
    APP_LOG(TS_OFF, VLEVEL_M, "pass \"%s\"\n", pass);
    APP_LOG(TS_OFF, VLEVEL_M, "User Config http://%s/\n", ip);
    APP_LOG(TS_OFF, VLEVEL_M, "WiFi MAC \"%s\"\n", mac);

    while (1);
  }
  
  // get the current user config
  // NOTE needed to configure teh AP ssid
  const UserConfiguration* cfg = UserConfigGet();
  APP_LOG(TS_OFF, VLEVEL_M, "Current user configuration:\n");
  UserConfigPrint();

  // start user config interface
  snprintf(ssid, sizeof(ssid), "ents-%d", cfg->logger_id);
  ControllerWiFiHost(ssid, pass);

  // Get host info
  ControllerWiFiHostInfo(ssid, ip, mac);
  APP_LOG(TS_OFF, VLEVEL_M, "ssid \"%s\"\n", ssid);
  APP_LOG(TS_OFF, VLEVEL_M, "pass \"%s\"\n", pass);
  APP_LOG(TS_OFF, VLEVEL_M, "User Config http://%s/\n", ip);
  APP_LOG(TS_OFF, VLEVEL_M, "WiFi MAC \"%s\"\n", mac);

  // Get Config from esp32
  APP_LOG(TS_OFF, VLEVEL_M, "Requesting configuration from ESP32...\r\n");
  UserConfigStatus status = ControllerUserConfigRequest();

  if (status == USERCONFIG_EMPTY_CONFIG || status != USERCONFIG_OK) {
    // 2. If ESP32 has empty config or request failed, send our config
    APP_LOG(TS_OFF, VLEVEL_M, "Sending FRAM configuration to ESP32...\r\n");
    status = ControllerUserConfigSend();

    if (status != USERCONFIG_OK) {
      APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config to ESP32: %d\r\n",
              status);
    }
  } else {
    // TODO Add user config save
    
    // Reload user config from FRAM
    if (UserConfigLoad() != USERCONFIG_OK) {
      APP_LOG(TS_OFF, VLEVEL_M, "Error loading saved configuration!\n");
      APP_LOG(TS_OFF, VLEVEL_M, "Try sending configuration again.\n");

      while (1);
    }
 
    // Print updated config
    APP_LOG(TS_OFF, VLEVEL_M, "Updated user configuration:\n");
    UserConfigPrint();
  }

  // required for SDI-12
  MX_USART2_UART_Init();
  MX_TIM1_Init();

  // initialize the user config interrupt
  //UserConfig_InitAdvanceTrace();
  
  // currently not functional
  //FIFO_Init();

  // Debug message, gets printed after init code
  APP_PRINTF("Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n", __DATE__, __TIME__);
  APP_PRINTF("Git SHA: %s\n", GIT_REV);
  
  // init senors interface
  SensorsInit();

  // configure enabled sensors
  for (int i=0; i < cfg->enabled_sensors_count; i++) {
    EnabledSensor sensor = cfg->enabled_sensors[i];
    if ((sensor == EnabledSensor_Voltage) || (sensor == EnabledSensor_Current)) {
      ADC_init();
      SensorsAdd(ADC_measure);
      APP_LOG(TS_OFF, VLEVEL_M, "ADS Enabled!\n");
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
}

