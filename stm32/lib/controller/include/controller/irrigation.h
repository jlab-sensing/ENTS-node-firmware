/**
 * @file irrigation.h
 * @date 2025-08-19
 * @author John Madden
 * @brief Irrigation interface implementation with the esp32
 */
#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_IRRIGATION_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_IRRIGATION_H_

#include "soil_power_sensor.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup controller
 * @defgroup controllerIrrigation Irrigation
 * @brief Irrigation interface for the esp32
 *
 * Requests the current state from the esp32 and updates the gpio state
 * accodingly.
 *
 * @{
 */

IrrigationCommand_State ControllerIrrigationCheck(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_WIFI_H_
