#include "wifi.h"

#include <stm32_systime.h>

#include "sys_app.h"
#include "sensors.h"
#include "stm32_seq.h"
#include "stm32_timer.h"
#include "fifo.h"
#include "controller/wifi.h"
#include "userConfig.h"
#include "status_led.h"

/**
 * @brief Timer for uploads
 * 
 */
static UTIL_TIMER_Object_t UploadTimer = {};


/**
 * @brief Function call for upload event
 * 
 * @param context Timer context.
 */
void UploadEvent(void *context);

/**
 * @brief Upload data to hub
 * 
 * Posts data to configured url.
 */
void Upload(void);

/**
 * @brief Connect to WiFi
 *
 * Connects to WiFi network and sets the system time. Code will block and
 * reattempt connection until successful.
 *
 * @return error status, false if successful, true if error
 */
bool Connect(void);

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
 * @brief Call initialization functions for ESP32
 *
 * Connects to a WiFi network, checks API health, and syncs time.
 */
void Esp32Init(void);

void WiFiInit(void) {
  APP_LOG(TS_OFF, VLEVEL_M, "WiFi app starting\r\n");

  // init esp32 WiFi code
  Esp32Init();

  // start timers for uploading
  StartUploads();
}

void UploadEvent(void *context) {
  UTIL_SEQ_SetTask((1 << CFG_SEQ_Task_WiFiUpload), CFG_SEQ_Prio_0);
}

void Upload(void) {
  const size_t buffer_size = 256;
  size_t buffer_len = 0;
  uint8_t buffer[buffer_size];

  // get buffer data
  FramStatus status = FramGet(buffer, &buffer_len);
  if (status != FRAM_OK) {
    if (status == FRAM_BUFFER_EMPTY) {
      APP_LOG(TS_OFF, VLEVEL_M, "Buffer empty!\r\n")
    } else {
      APP_LOG(TS_OFF, VLEVEL_M,
          "Error getting data from fram buffer. FramStatus = %d\r\n", status);
    }
    return;
  }

  // print buffer
  APP_LOG(TS_ON, VLEVEL_M, "Payload[%d]: ", buffer_len);
  for (int i = 0; i < buffer_len; i++)
  {
    APP_LOG(TS_OFF, VLEVEL_M, "%x ", buffer[i]);
  }
  APP_LOG(TS_OFF, VLEVEL_M, "\r\n");

  // posts data to website
  APP_LOG(TS_ON, VLEVEL_M, "Uploading data...\t");
  uint8_t resp[256];
  size_t resp_len = 0;
  int http_code = ControllerWiFiPost(buffer, buffer_len, resp, &resp_len);
  APP_LOG(TS_OFF, VLEVEL_M, "%d\r\n", http_code);
  
  // reconnect and retry if general error
  if (http_code != 200) {
    APP_LOG(TS_ON, VLEVEL_M, "Error with WiFi connection!\r\n");
    APP_LOG(TS_ON, VLEVEL_M, "Attempting Reconnect before next upload\r\n");
    
    Connect();
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
  UTIL_TIMER_Create(&UploadTimer, UploadPeriod, UTIL_TIMER_PERIODIC, UploadEvent, NULL);
  UTIL_TIMER_Start(&UploadTimer);
  APP_LOG(TS_OFF, VLEVEL_M, "Started!\r\n");
}

bool Connect(void) {
  //  load configurations
  const UserConfiguration* cfg = UserConfigGet();

  const char* ssid = cfg->WiFi_SSID;
  const char* passwd = cfg->WiFi_Password;
 
  APP_LOG(TS_OFF, VLEVEL_M, "Connecting to %s. Status: ", ssid);
  uint8_t wifi_status = ControllerWiFiInit(ssid, passwd);
  APP_LOG(TS_OFF, VLEVEL_M, "%d\r\n", wifi_status);

  // check for errors with the following order
  // WL_NO_SSID_AVAIL
  // WL_CONNECT_FAILED
  // WL_CONNECT_LOST
  // WL_NO_SHIELD
  if (wifi_status == 1 || wifi_status == 4 || wifi_status == 5 || wifi_status == 255) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error connecting to WiFi!\r\n");
    return true;
  }

  return false;
}

bool TimeSync(void) {
  SysTime_t ts = {.Seconds = -1, .SubSeconds = 0};

  APP_LOG(TS_OFF, VLEVEL_M, "Syncing time...\t");
  ts.Seconds = ControllerWiFiTime();
 
  // check for errors
  if (ts.Seconds == 0) {
    APP_LOG(TS_OFF, VLEVEL_M, "Error syncing time!\r\n");
    return true;
  }
  
  SysTimeSet(ts);

  APP_LOG(TS_OFF, VLEVEL_M, "Done!\r\n");
  APP_LOG(TS_OFF, VLEVEL_M, "Current timestamp is %d\r\n", ts.Seconds);

  return false;
}

bool Check(void) {
  const UserConfiguration* cfg = UserConfigGet();
  
  //const char url[] = "dirtviz.jlab.ucsc.edu";
  //const uint32_t port = 80;
  
  const char* url = cfg->API_Endpoint_URL;
  const uint32_t port = cfg->API_Endpoint_Port; 

  // retry pinging API
  // http code is misleading as it holds http code and the WiFi status
  unsigned int http_code = 0;
  while (http_code != 200) {
    APP_LOG(TS_OFF, VLEVEL_M, "Checking API health...\t");
    http_code = ControllerWiFiCheck(url, port);
    APP_LOG(TS_OFF, VLEVEL_M, "%d\r\n", http_code);

    // check for WiFi issues
    if (http_code == 1 || http_code == 4 || http_code == 5) {
      return true;
    }

    HAL_Delay(5000);
  }

  return false;
}

void Esp32Init(void) {
  bool error = true;

  while (error) {
    // Start WiFi connection
    error = Connect();
    if (error) continue;

    // Check WiFi connection and API health
    error = Check();
    if (error) continue;

    // sync with NTP server
    error = TimeSync();
    if (error) continue;
  }
}
