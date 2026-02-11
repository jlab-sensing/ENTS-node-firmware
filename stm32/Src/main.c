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

  HAL_Delay(100);

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
  APP_PRINTF("Soil Power Sensor Wio-E5 firmware, compiled on %s %s\n", __DATE__,
             __TIME__);
  APP_PRINTF("Git SHA: %s\n\n", GIT_REV);

  // Typically, you need to apply a userconfig that specifies LoRa as the upload method before you can see the LoRaWAN keys.
  // To eliminate that step in the board setup process, the only uniquely changing values (DevAddr and DevEUI) are shown.
  // To keep in line with the LmHandler's formatting, the common AppKey, NwkKey, AppSKey, and NwkSKey are also shown.
  uint32_t devAddr = 0;
  GetDevAddr(&devAddr);
  APP_PRINTF(
      "###### AppKey:      2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C\r\n"
      "###### NwkKey:      2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C\r\n"
      "###### AppSKey:     2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C\r\n"
      "###### NwkSKey:     2B:7E:15:16:28:AE:D2:A6:AB:F7:15:88:09:CF:4F:3C\r\n"
      "###### DevEUI:      00:80:E1:15:%02X:%02X:%02X:%02X\r\n"
      "###### AppEUI:      01:01:01:01:01:01:01:01\r\n"
      "###### DevAddr:     %02X:%02X:%02X:%02X\r\n",
      (devAddr >> 24) & 0xFF, (devAddr >> 16) & 0xFF, (devAddr >> 8) & 0xFF,
      (devAddr) & 0xFF, (devAddr >> 24) & 0xFF, (devAddr >> 16) & 0xFF,
      (devAddr >> 8) & 0xFF, (devAddr) & 0xFF);

  // Start status LEDs
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
      APP_LOG(TS_OFF, VLEVEL_M, "BME280 Enabled!\n");
    }
    // if (sensor == EnabledSensor_Phytos31) {
    //   Phytos31Init();
    //   SensorsAdd(Phytos31_measure);
    //   APP_LOG(TS_OFF, VLEVEL_M, "Phytos31 Enabled!\n");
    // }
    // if (sensor == EnabledSensor_SEN0308) {
    //   CapSoilInit();
    //   SensorsAdd(SEN0308_measure);
    //   APP_LOG(TS_OFF, VLEVEL_M, "SEN0308 Cap Soil Sensor Enabled!\n");
    // }
    // if (sensor == EnabledSensor_SEN0257) {
    //   PressureInit();
    //   SensorsAdd(WatPress_measure);
    //   APP_LOG(TS_OFF, VLEVEL_M, "SEN0257 Water Pressure Sensor Enabled!\n");
    // }
    // if (sensor == EnabledSensor_YFS210C) {
    //   FlowInit();
    //   SensorsAdd(WatFlow_measure);
    //   APP_LOG(TS_OFF, VLEVEL_M, "YFS210C Flow Meter Enabled!\n");
    // }
    if (sensor == EnabledSensor_PCAP02) {
      pcap02_init();
      SensorsAdd(pcap02_measure);
      APP_LOG(TS_OFF, VLEVEL_M, "PCAP02 Enabled!\n");
    }
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
    FlowGetMeasurement();  // This updates the internal state
    last_check = current_time;
  }
}
#endif
