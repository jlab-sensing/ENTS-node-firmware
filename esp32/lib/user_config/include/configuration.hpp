#ifndef LIB_USER_CONFIG_INCLUDE_CONFIGURATION_H
#define LIB_USER_CONFIG_INCLUDE_CONFIGURATION_H

#include <stdint.h>

#include <Arduino.h>

#include "soil_power_sensor.pb.h"

/**
 * @brief Set the user configuration.
 *
 * Performs a deep copy of the configuration.
 */
void setConfig(const UserConfiguration &new_config);

/**
 * @brief Get a copy of the current user configuration.
 *
 * This function returns a SHALLOW coyy of the user config. No modification
 * should be made.
 *
 * Checks are also performed to ensure the configuration can be
 * encoded/decoded.
 *
 * @return A copy of the current user configuration.
 */
const UserConfiguration& getConfig();

/**
 * @brief Get the current configuration as a JSON string.
 *
 * This function converts the current user configuration to a JSON string
 * representation.
 *
 * @return A JSON string representing the current user configuration.
 */
String getConfigJson();

/**
 * @brief Print the current user configuration to the log.
 */
void printReceivedConfig();

#endif  // LIB_USER_CONFIG_INCLUDE_CONFIGURATION_H
