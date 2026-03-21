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

// peripherials
#include "app_lorawan.h"

// userland
#include "adc.h"
#include "ads.h"
#include "bme280_sensor.h"
#include "board.h"
#include "controller/controller.h"
#include "controller/power.h"
#include "controller/wifi.h"
#include "controller/wifi_userconfig.h"
#include "pcap02.h"
#include "phytos31.h"
#include "sen0308.h"
#include "sensors.h"
#include "status_led.h"
#include "teros12.h"
#include "teros21.h"
#include "userConfig.h"
#include "user_config.h"
#include "waterFlow.h"
#include "waterPressure.h"
#include "wifi.h"

// Board configuration - define ONLY ONE of these
// Comment these out to disable sensors
#define DEFAULT
// #define USE_CAP_SOIL_SENSOR
// #define USE_WATER_PRESSURE_SENSOR
// #define USE_FLOW_METER_SENSOR

/** Fast sample/upload interval in milliseconds when switch is ON */
#define FAST_INTERVAL_MS 5000U

/** How often to poll the switch in the main loop (ms) */
#define SWITCH_POLL_INTERVAL_MS 100U

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

  // Configure PA10 as regular input with internal pull-up
  // External 10kΩ resistor to 3.3V provides stronger pull-up
  // Switch open       -> PA10 HIGH (normal interval from user config)
  // Switch closed GND -> PA10 LOW  (fast interval = FAST_INTERVAL_MS)
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  __HAL_RCC_GPIOA_CLK_ENABLE();
  GPIO_InitStruct.Pin  = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  StatusLedInit();
  StatusLedFlashSlow();

  // Wakeup via GPIO pin
  ControllerWakeup();

  // initialize esp32 controller module
  ControllerInit();

  if (!ControllerPowerWakeup()) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error waking up ESP32!\n");
  }

  // Print warning when using TEST_USER_CONFIG
#ifdef TEST_USER_CONFIG
  APP_LOG(TS_OFF, VLEVEL_M, "WARNING: TEST_USER_CONFIG is enabled!\n");
#endif  // TEST_USER_CONFIG

  UserConfigStart(120);
  const UserConfiguration* cfg = UserConfigGet();

  // initialize the user config interrupt
  // UserConfig_InitAdvanceTrace();

  // currently not functional
  // FIFO_Init();

  APP_LOG(TS_OFF, VLEVEL_M, "Enabling Sensors\n");
  APP_LOG(TS_OFF, VLEVEL_M, "----------------\n");

  // init senors interface
  SensorsInit();

  // configure enabled sensors
  for (int i = 0; i < cfg->enabled_sensors_count; i++) {
    EnabledSensor sensor = cfg->enabled_sensors[i];
    if (sensor == EnabledSensor_Voltage) {
#ifdef DEFAULT
      ADC_init();
      SensorsAdd(ADC_measureVoltage);
      APP_LOG(TS_OFF, VLEVEL_M, "Voltage Enabled!\n");
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

#ifdef USE_PHYTOS31_SENSOR

#endif
    }
    if (sensor == EnabledSensor_Current) {
      ADC_init();
      SensorsAdd(ADC_measureCurrent);
      APP_LOG(TS_OFF, VLEVEL_M, "Current Enabled!\n");
    }
    if (sensor == EnabledSensor_Teros12) {
      APP_LOG(TS_OFF, VLEVEL_M, "Teros12 Enabled!\n");
      SensorsAdd(Teros12Measure);
    }
    if (sensor == EnabledSensor_Teros21) {
      SensorsAdd(Teros21Measure);
      APP_LOG(TS_OFF, VLEVEL_M, "Teros21 Enabled!\n");
    }
    if (sensor == EnabledSensor_BME280) {
      BME280Init();
      SensorsAdd(BME280Measure);
    }
    if (sensor == EnabledSensor_PCAP02) {
      pcap02_init();
      SensorsAdd(pcap02_measure);
    }
  }

  APP_LOG(TS_OFF, VLEVEL_M, "\r\n=== Switch polling mode ===\r\n");
  APP_LOG(TS_OFF, VLEVEL_M, "Normal interval : %u ms\r\n", cfg->Upload_interval * 1000);
  APP_LOG(TS_OFF, VLEVEL_M, "Fast interval   : %u ms\r\n", FAST_INTERVAL_MS);
  APP_LOG(TS_OFF, VLEVEL_M, "Poll interval   : %u ms\r\n", SWITCH_POLL_INTERVAL_MS);
  APP_LOG(TS_OFF, VLEVEL_M, "PA10 initial    : %d (1=open 0=closed)\r\n",
          HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10));
  APP_LOG(TS_OFF, VLEVEL_M, "===========================\r\n\r\n");

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

  static bool last_fast_mode = false;
  static uint32_t last_poll = 0;

  while (1) {
    MX_LoRaWAN_Process();

#ifdef USE_FLOW_METER_SENSOR
    FlowBackgroundTask();
#endif

    uint32_t now = HAL_GetTick();
    if (now - last_poll >= SWITCH_POLL_INTERVAL_MS) {
      last_poll = now;

      bool fast_mode = (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET);

      // Always print so we can see exactly what the pin reads
      APP_LOG(TS_OFF, VLEVEL_M,
              "[POLL] PA10=%d  fast_mode=%d  last_fast_mode=%d\r\n",
              HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10),
              (int)fast_mode,
              (int)last_fast_mode);

      if (fast_mode != last_fast_mode) {
        if (fast_mode) {
          APP_LOG(TS_OFF, VLEVEL_M,
                  ">>> Switch ON  -> fast mode (%u ms)\r\n", FAST_INTERVAL_MS);
          SensorsChangePeriod(FAST_INTERVAL_MS);
          WiFiChangePeriod(FAST_INTERVAL_MS);
        } else {
          uint32_t normal_ms = cfg->Upload_interval * 1000;
          APP_LOG(TS_OFF, VLEVEL_M,
                  ">>> Switch OFF -> normal mode (%u ms)\r\n", normal_ms);
          SensorsChangePeriod(normal_ms);
          WiFiChangePeriod(normal_ms);
        }
        last_fast_mode = fast_mode;
      }
    }
  }
}

// TODO update this to use the scheduler
#ifdef USE_FLOW_METER_SENSOR
void FlowBackgroundTask(void) {
  static uint32_t last_check = 0;
  uint32_t current_time = HAL_GetTick();

  // Update flow measurement every 100ms
  if (current_time - last_check >= 100) {
    FlowGetMeasurement();
    last_check = current_time;
  }
}
#endif
