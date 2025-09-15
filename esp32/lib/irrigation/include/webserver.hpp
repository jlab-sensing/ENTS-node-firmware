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

// Auto irrigation control functions
void EnableAutoIrrigation(float min_thresh, float max_thresh);
void DisableAutoIrrigation();
bool IsAutoIrrigationEnabled();
float GetMinThreshold();
float GetMaxThreshold();
unsigned long GetCheckInterval();
void SetCheckInterval(unsigned long interval_ms);
float GetSimpleSoilMoisture();
void CheckIrrigationConditions();
float GetCurrentMoistureFromCache();
