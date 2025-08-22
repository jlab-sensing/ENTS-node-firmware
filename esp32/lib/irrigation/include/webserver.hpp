#ifndef LIB_IRRIGATION_INCLUDE_WEBSERVER_HPP
#define LIB_IRRIGATION_INCLUDE_WEBSERVER_HPP

#include "soil_power_sensor.pb.h"

/**
 * @brief Setup endpoints for the webserver
 */
void SetupServer();

/**
 * @brief Handle requests from the webserver
 */
void HandleClient();

#endif  // LIB_IRRIGATION_INCLUDE_WEBSERVER_HPP

/**
 * @brief Get the current solenoid state
 */
IrrigationCommand_State GetSolenoidState();

/**
 * @brief Set the solenoid state
 */
void SetSolenoidState(IrrigationCommand_State newState);