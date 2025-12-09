/**
 * @file power.h
 * @date 2025-11-04
 * @author John Madden <jmadden173@pm.me>
 * @brief Power interface for the esp32
 */

#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_POWER_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_POWER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup controller
 * @defgroup controllerPower Power
 * @brief Power interface for the esp32
 */

bool ControllerPowerSleep(void);

bool ControllerPowerWakeup(void);

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_POWER_H_
