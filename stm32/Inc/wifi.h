/**
 * @file wifi.h
 * @author John Madden (jmadden173@pm.me)
 * @brief 
 * @version 0.1
 * @date 2024-10-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef INC_WIFI_H_
#define INC_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sys_app.h"

/**
 * @brief Initialize WiFi uploading method
 *
 * Connect to WiFi network, sync time, check API health, and start upload
 * timer.
 */
void WiFiInit(void);

#ifdef __cplusplus
}
#endif

#endif  // INC_WIFI_H_
