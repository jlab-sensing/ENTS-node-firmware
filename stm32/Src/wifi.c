#include "wifi.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32_systime.h>

#include "adc.h"
#include "ads.h"
#include "app_lorawan.h"
#include "bme280_sensor.h"
#include "board.h"
#include "controller/controller.h"
#include "controller/wifi.h"
#include "dma.h"
#include "fifo.h"
#include "gpio.h"
#include "i2c.h"
#include "main.h"
#include "phytos31.h"
#include "rtc.h"
#include "sensors.h"
#include "status_led.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "sys_app.h"
#include "teros12.h"
#include "teros21.h"
#include "tim.h"
#include "usart.h"
#include "userConfig.h"
#include "user_config.h"
#include "wifi.h"

#include "payload.h"

/**
 * @brief Timer for uploads
 *
 */
static UTIL_TIMER_Object_t UploadTimer = {};

/**
 * @brief Maximum number of retries for any event
 */
const unsigned int max_retries = 5;

/**
 * @brief Delay in milliseconds between retries
 */
const unsigned int retry_delay = 1000;

/**
 * @brief Function call for upload event
 *
 * @param context Timer context.
 */
void UploadEvent(void* context);

/**
 * @brief Upload data to hub
 *
 * Posts data to configured url.
 */
void Upload(void);

/**
 * @brief Connect to WiFi
 *
 * Connects to WiFi network and sets the system time.
 *
 * @return error status, false if successful, true if error
 */
bool Connect(void);

/**
 * @brief Disconnect from WiFi
 *
 * @return error status, false if successful, true if error
 */
bool Disconnect(void);

/**
 * @brief Timesync with NTP server
 *
 * @return error status, false if successful, true if error
 */
bool TimeSync(void);

/**
 * @brief Check API health
 *
 * @return error status, false if successful, true if error
 */
bool Check(void);

/**
 * @brief Setup upload timer
 *
 * Start sensor measurements and setup upload task/timer.
 */
void StartUploads(void);

/**
 * @brief Stop upload timer
 */
void StopUploads(void);

/**
 * @brief Resumes upload timer
 */
void ResumeUploads(void);

/**
 * @brief Error handler
 *
 * Turns on LEDs, waits for a reconnection, and resumes uploads.
 */
void ErrorHandler(void);

/**
 * @brief Call initialization functions for ESP32
 *
 * Connects to a WiFi network, checks API health, and syncs time.
 *
 * @returns true if successful, false if error
 */
bool Esp32Init(void);

void WiFiInit(void) {
  APP_LOG(TS_OFF, VLEVEL_M, "WiFi app starting\r\n");

  // loop until sucessful connection
  while (!Esp32Init()) {
    Disconnect();
  }

  // start timers for uploading
  StartUploads();
}

void UploadEvent(void* context) {
   // lower priority (higher value) than measure task
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_WiFiUpload), CFG_SEQ_Prio_1); 
}

void Upload(void) {
  const size_t buffer_size = 256;
  size_t buffer_len = 0;
  uint8_t buffer[buffer_size];

  // get payload (pegged at 512)
  PayloadStatus payload_status = PAYLOAD_OK;
  payload_status = FormatPayload(buffer, buffer_size, &buffer_len);
  if (payload_status == PAYLOAD_ERROR) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error formatting payload\r\n");
    return;
  } else if (payload_status == PAYLOAD_NO_DATA) {
    APP_LOG(TS_OFF, VLEVEL_M, "No data to send\r\n");
    return;
  }

  // print buffer
  APP_LOG(TS_ON, VLEVEL_M, "Payload[%d]: ", buffer_len);
  for (int i = 0; i < buffer_len; i++) {
    APP_LOG(TS_OFF, VLEVEL_M, "%x ", buffer[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

  // posts data to website
  APP_LOG(TS_ON, VLEVEL_M, "Uploading data.");
  if (!ControllerWiFiPost(buffer, buffer_len)) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error! Could not communicate with esp32!\r\n");
  }

  for (unsigned int retries = 0;; retries++) {
    HAL_Delay(retry_delay);
    APP_LOG(TS_OFF, VLEVEL_M, ".");

    ControllerWiFiResponse resp = ControllerWiFiCheckRequest();
    APP_LOG(TS_OFF, VLEVEL_M, "(%d) \r\n", resp.http_code);

    if (resp.http_code == 200) {
      break;
    } else {
      // check category of error
      if (resp.http_code == 0) {
        APP_LOG(TS_OFF, VLEVEL_M,
                "Error when posting data! Likely error with WiFi or response "
                "timeout.\r\n");
      } else {
        APP_LOG(TS_OFF, VLEVEL_M,
                "Error with HTTP code! Likely error with measurement format or "
                "backend.\r\n");
      }

      // give up after max retries and tirgger error handler
      if (retries >= max_retries) {
        APP_LOG(TS_ON, VLEVEL_M, "Max retries reached! Stopping upload.\r\n");
        ErrorHandler();
        return;
      }
    }
  }

  if (FramBufferLen() > 0) {
    APP_LOG(TS_ON, VLEVEL_M, "Buffer not empty, starting another upload\r\n");
    UploadEvent(NULL);
  }

  StatusLedOff();
}

void StartUploads(void) {
  // start sensor measurements
  APP_LOG(TS_ON, VLEVEL_M, "Starting sensor measurements...\t");
  SensorsStart();
  APP_LOG(TS_OFF, VLEVEL_M, "Started!\r\n");

  // get upload period from user config
  const UserConfiguration* cfg = UserConfigGet();
  UTIL_TIMER_Time_t UploadPeriod = cfg->Upload_interval * 1000;

  // setup upload task
  APP_LOG(TS_ON, VLEVEL_M, "Starting upload task...\t")
  UTIL_SEQ_RegTask((1 << CFG_SEQ_Task_WiFiUpload), UTIL_SEQ_RFU, Upload);
  UploadEvent(NULL);
  UTIL_TIMER_Create(&UploadTimer, UploadPeriod, UTIL_TIMER_PERIODIC,
                    UploadEvent, NULL);
  UTIL_TIMER_Start(&UploadTimer);
  APP_LOG(TS_OFF, VLEVEL_M, "Started!\r\n");
}

void StopUploads(void) {
  // stop upload timer
  APP_LOG(TS_ON, VLEVEL_M, "Stopping upload task...\t");
  UTIL_TIMER_Stop(&UploadTimer);
  APP_LOG(TS_OFF, VLEVEL_M, "Stopped!\r\n");
}

void ResumeUploads(void) {
  // stop upload timer
  APP_LOG(TS_ON, VLEVEL_M, "Starting upload task...\t")
  UTIL_TIMER_Start(&UploadTimer);
  APP_LOG(TS_OFF, VLEVEL_M, "Started!\r\n");
}

void ErrorHandler(void) {
  // turn on status LED
  StatusLedOn();

  StopUploads();

  // first disconnect, then retry until connected to wifi
  do {
    Disconnect();
  } while (!Esp32Init());

  ResumeUploads();
}

bool Connect(void) {
  //  load configurations
  const UserConfiguration* cfg = UserConfigGet();

  const char* ssid = cfg->WiFi_SSID;
  const char* passwd = cfg->WiFi_Password;

  APP_LOG(TS_ON, VLEVEL_M, "Connecting to %s.", ssid);
  if (!ControllerWiFiConnect(ssid, passwd)) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error! Could not communicate with esp32!\r\n");
    return false;
  }

  for (unsigned int retries = 0;; retries++) {
    HAL_Delay(retry_delay);
    APP_LOG(TS_OFF, VLEVEL_M, ".")

    ControllerWiFiStatus status = ControllerWiFiCheckWiFi();
    if (status == CONTROLLER_WIFI_CONNECTED) {
      break;
    } else {
      if (retries >= max_retries) {
        APP_LOG(TS_OFF, VLEVEL_M, "Error! Timeout after %d retries!\r\n",
                retries);
        return false;
      } else {
        if (status == CONTROLLER_WIFI_DISCONNECTED) {
          continue;
        } else if (status == CONTROLLER_WIFI_NO_SSID_AVAIL) {
          APP_LOG(TS_OFF, VLEVEL_M, "Error! No SSID available!\r\n");
          return false;
        } else if (status == CONTROLLER_WIFI_CONNECT_FAILED) {
          APP_LOG(TS_OFF, VLEVEL_M, "Error! Connect failed!\r\n");
          return false;
        }
      }
    }
  }

  APP_LOG(TS_OFF, VLEVEL_M, "Connected!\r\n");

  return true;
}

bool Disconnect(void) {
  APP_LOG(TS_ON, VLEVEL_M, "Disconnecting from WiFi.");
  if (!ControllerWiFiDisconnect()) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error! Could not communicate with esp32!\r\n");
    return false;
  }

  for (unsigned int retries = 0;; retries++) {
    HAL_Delay(retry_delay);
    APP_LOG(TS_OFF, VLEVEL_M, ".")

    ControllerWiFiStatus status = ControllerWiFiCheckWiFi();
    if (status == CONTROLLER_WIFI_IDLE_STATUS) {
      break;
    } else if (status == CONTROLLER_WIFI_DISCONNECTED) {
      break;
    } else {
      if (retries >= max_retries) {
        APP_LOG(TS_OFF, VLEVEL_M, "Error! Timeout after %d retries!\r\n",
                retries);
        return false;
      }
    }
  }
  APP_LOG(TS_OFF, VLEVEL_M, "Done!\r\n");

  return true;
}

bool TimeSync(void) {
  SysTime_t ts = {.Seconds = -1, .SubSeconds = 0};

  APP_LOG(TS_OFF, VLEVEL_M, "Syncing time.");
  if (!ControllerWiFiNtpSync()) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error! Could not communicate with esp32!\r\n");
    return false;
  }

  for (unsigned int retries = 0;; retries++) {
    HAL_Delay(retry_delay);
    APP_LOG(TS_OFF, VLEVEL_M, ".");

    ts.Seconds = ControllerWiFiTime();

    // check for errors
    if (ts.Seconds != 0) {
      break;
    } else {
      if (retries >= max_retries) {
        APP_LOG(TS_OFF, VLEVEL_M, "Error! Timeout after %d retries!\r\n",
                retries);
        return false;
      }
    }
  }

  SysTimeSet(ts);

  APP_LOG(TS_OFF, VLEVEL_M, "Done!\r\n");
  APP_LOG(TS_OFF, VLEVEL_M, "Current timestamp is %d\r\n", ts.Seconds);

  return true;
}

bool Check(void) {
  const UserConfiguration* cfg = UserConfigGet();

  // const char url[] = "dirtviz.jlab.ucsc.edu";
  // const uint32_t port = 80;

  const char* url = cfg->API_Endpoint_URL;

  APP_LOG(TS_OFF, VLEVEL_M, "Checking API health.");
  if (!ControllerWiFiCheckApi(url)) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error! Could not communicate with esp32!\r\n");
    return false;
  }

  for (unsigned int retries = 0;; retries++) {
    APP_LOG(TS_OFF, VLEVEL_M, ".");

    ControllerWiFiResponse resp = ControllerWiFiCheckRequest();
    APP_LOG(TS_OFF, VLEVEL_M, "(%d) \r\n", resp.http_code);

    if (resp.http_code == 200) {
      break;
    } else {
      // check category of error
      if (resp.http_code == 0) {
        APP_LOG(TS_OFF, VLEVEL_M,
                "Error when posting data! Likely error with WiFi or response "
                "timeout.\r\n");
      } else {
        APP_LOG(TS_OFF, VLEVEL_M,
                "Error with HTTP code! Likely error with measurement format or "
                "backend.\r\n");
      }

      // give up after max retries and tirgger error handler
      if (retries >= max_retries) {
        APP_LOG(TS_ON, VLEVEL_M, "Max retries reached! Stopping upload.\r\n");
        return false;
      }
    }

    HAL_Delay(retry_delay);
  }

  return true;
}

bool Esp32Init(void) {
  // Start WiFi connection
  if (!Connect()) {
    return false;
  }

  // Check WiFi connection and API health
  if (!Check()) {
    return false;
  }

  // sync with NTP server
  if (!TimeSync()) {
    return false;
  }

  // Stop webserver if running (timeout of 60 seconds)
  UserConfigSetupStop(60);

  return true;
}
