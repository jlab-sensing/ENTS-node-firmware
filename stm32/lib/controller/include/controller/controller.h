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
 * @brief Trigger esp32 wakeup pin
 *
 * After this function is called and the controller is initialized with
 * ControllerInit(), the function ControllerPowerWakeup() should be used to
 * check that the esp32 has correctly woken up.
 *
 * Since this is blocking code, a small internal delay is added for the startup
 * time on the esp32.
 */
void ControllerWakeup(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_INCLUDE_CONTROLLER_CONTROLLER_H_
