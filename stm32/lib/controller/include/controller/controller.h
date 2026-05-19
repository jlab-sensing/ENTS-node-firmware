/**
 * @file controller.h
 * @date 2024-10-13
 * @author John Madden <jmadden173@pm.me>
 * @brief Controller module for stm32
 *
 */

/**
 * @ingroup stm32
 * @defgroup controller Controller
 * @brief Controller library for communication between stm32 and esp32
 */

#ifndef LIB_CONTROLLER_INCLUDE_CONTROLLER_CONTROLLER_H_
#define LIB_CONTROLLER_INCLUDE_CONTROLLER_CONTROLLER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup controller
 * @defgroup controllerInterface Controller Interface
 * @brief Interface for the controller module
 * @{
 */

/**
 * @brief Shared initialization for all esp32  modules
 *
 * Allocates memory to the tx and rx buffers.
 *
 * @todo Add check for communication with the esp32
 */
void ControllerInit(void);

/**
 * @brief Shared deinitialize for all esp32 modules
 *
 * Free memory associated with the tx and rx buffers.
 */
void ControllerDeinit(void);

/**
 * @brief Wake up ESP32 from sleep state
 * @pre ESP32 must be enabled in order to wake it up, see `ControllerDeviceEnable()`.
 * 
 * @note `ControllerTransaction()` calls this function (TODO) in order to wake the
 * ESP32 so it can receive the transmission and reply (then go to sleep).
 *
 * A small internal delay is added for the startup time on the esp32.
 * 
 * Normally, the ESP32 enters the sleep state after providing data in response to
 * the STM32 calling `ControllerReceive()`.
 */
void ControllerWakeup(void);

/**
 * @brief Enable the ESP32 EN pin.
 */
void ControllerDeviceEnable(void);

/**
 * @brief Disable the ESP32 EN pin
 */
void ControllerDeviceDisable(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_CONTROLLER_H_
