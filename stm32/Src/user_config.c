#include "user_config.h"

#include "sys_app.h"
#include "userConfig.h"
#include "controller/wifi.h"
#include "controller/wifi_userconfig.h"
#include "controller/power.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "utilities_def.h"

static UTIL_TIMER_Object_t UserConfigTimer = {};

void UserConfigStopEvent(void* context);

void UserConfigStop(void);

void UserConfigStart(unsigned int timeout) {
  // variables to store WiFi host info
  char ssid[255] = {};
  char ip[16] = {};
  char mac[18] = {};
 
  // constant password for AP
  const char pass[] = "ilovedirt";


  // Reload user config from FRAM 
  UserConfigStatus status_load = UserConfigLoad();

  // start user config interface
  if (status_load == USERCONFIG_OK) {
    // print current user config
    APP_LOG(TS_OFF, VLEVEL_M, "\nCurrent user configuration:\n");
    APP_LOG(TS_OFF, VLEVEL_M, "---------------------------\n");
    UserConfigPrint();
    APP_LOG(TS_OFF, VLEVEL_M, "\n");
  }
  
  uint32_t devAddr = 0;
  GetDevAddr(&devAddr);
  snprintf(ssid, sizeof(ssid), "ents-%08X", devAddr);
  
  ControllerWiFiHost(ssid, pass);
  ControllerUserConfigStart();

  // Get host info
  ControllerWiFiHostInfo(ssid, ip, mac, NULL);
  APP_LOG(TS_OFF, VLEVEL_M, "\nWiFi AP Info:\n");
  APP_LOG(TS_OFF, VLEVEL_M, "---------------\n");
  APP_LOG(TS_OFF, VLEVEL_M, "ssid \"%s\"\n", ssid);
  APP_LOG(TS_OFF, VLEVEL_M, "pass \"%s\"\n", pass);
  APP_LOG(TS_OFF, VLEVEL_M, "User Config http://%s/\n", ip);
  APP_LOG(TS_OFF, VLEVEL_M, "WiFi AP MAC: \"%s\"\n", mac);
  APP_LOG(TS_OFF, VLEVEL_M, "\n");
  
  // Get Config from esp32
  APP_LOG(TS_OFF, VLEVEL_M, "Requesting configuration from ESP32...\n");
  UserConfigStatus status = ControllerUserConfigRequest();

  // If esp32 responded with an empty config
  if (status == USERCONFIG_EMPTY_CONFIG || status != USERCONFIG_OK) {
    // Don't do anything if we don't have a saved config
    if (status_load != USERCONFIG_OK) {
      APP_LOG(TS_OFF, VLEVEL_M, "No configuration to send to ESP32!\n");
      // it's a trap! No valid userconfig
      // Waiting for new configuration on reset
      while (1);

    // Otherwise send the saved config and continue
    } else {
      // If ESP32 has empty config or request failed, send our config
      APP_LOG(TS_OFF, VLEVEL_M, "Sending FRAM configuration to ESP32...\n");
      status = ControllerUserConfigSend();

      if (status != USERCONFIG_OK) {
        APP_LOG(TS_OFF, VLEVEL_M, "Failed to send config to ESP32: %d\n",
            status);
      }
    }

  // if ESP32 provided a config
  } else { 
    // Reload user config from FRAM 
    if (UserConfigLoad() != USERCONFIG_OK) {
      APP_LOG(TS_OFF, VLEVEL_M, "Error saved configuration not valid!\n");
      APP_LOG(TS_OFF, VLEVEL_M, "Try sending configuration again.\n");

      while (1);
    }
 
    // Print updated config
    APP_LOG(TS_OFF, VLEVEL_M, "\nUpdated user configuration:\n");
    APP_LOG(TS_OFF, VLEVEL_M, "---------------------------\n");
    UserConfigPrint();
    APP_LOG(TS_OFF, VLEVEL_M, "\n");
  }
}

void UserConfigSetupStop(unsigned int timeout) {
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_UserConfigStop), UTIL_SEQ_RFU, UserConfigStop);
  UTIL_TIMER_Create(&UserConfigTimer, timeout * 1000, UTIL_TIMER_ONESHOT,
                    UserConfigStopEvent, NULL);
  UTIL_TIMER_Start(&UserConfigTimer);
}

void UserConfigStopEvent(void* context) {
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_UserConfigStop), CFG_SEQ_Prio_0);
}

void UserConfigStop(void) {
  APP_LOG(TS_ON, VLEVEL_M, "Stopping UserConfig webserver...\t");

  uint8_t clients = 0;
  ControllerWiFiHostInfo(NULL, NULL, NULL, &clients);

  if (clients > 0) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error! %d clients still connected!\n", clients);
    UTIL_TIMER_Start(&UserConfigTimer);
  } else {
    if (!ControllerWiFiStopHost()) {
      APP_LOG(TS_OFF, VLEVEL_M, "Error! Could not stop WiFi network!\n");
      UTIL_TIMER_Start(&UserConfigTimer);
    } else {
      APP_LOG(TS_OFF, VLEVEL_M, "Stopped!\n");

      // if uploading via LoRaWAN deep sleep esp32
      const UserConfiguration* cfg = UserConfigGet();
      if (cfg->Upload_method == Uploadmethod_LoRa) {
        APP_LOG(TS_OFF, VLEVEL_M, "Entering deep sleep...\n");
        ControllerPowerSleep();
      }
    } 
  }
}
