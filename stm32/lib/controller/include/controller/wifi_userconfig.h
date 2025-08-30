#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_

#include <stddef.h>
#include <stdint.h>

#include "communication.h"
#include "userConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Request user configuration from ESP32
 *
 * @return UserConfigStatus indicating success or failure
 */
UserConfigStatus ControllerUserConfigRequest(void);

/**
 * @brief Send user configuration to ESP32
 *
 * @return UserConfigStatus indicating success or failure
 */
UserConfigStatus ControllerUserConfigSend(void);

/**
 * @brief Start the user configuration website.
 *
 * This function must be called after the station has been setup.
 *
 * @return true if command was sent sucessfully, false otherwise
 */
bool ControllerUserConfigStart(void);

/**
 * @brief Check if configuration is empty/uninitialized
 *
 * @param config Pointer to UserConfiguration structure
 * @return true if config is empty, false otherwise
 */
bool isConfigEmpty(const UserConfiguration *config);

/**
 * @brief Print user configuration details to log
 *
 * @param config Pointer to UserConfiguration structure
 */
void printUserConfig(const UserConfiguration *config);

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
