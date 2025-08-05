/**
 * @file battery.h
 * @brief Provides declarations for battery monitoring interface.
 * @author John Madden <jmadden173@pm.me>
 * @date 2023-11-16
 */

#ifndef LIB_BATTERY_INCLUDE_BATTERY_H_
#define LIB_BATTERY_INCLUDE_BATTERY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32wlxx_hal.h"

/**
 * @ingroup stm32
 * @defgroup battery Battery (Not functional)
 * @brief Battery monitoring interface
 *
 * @note This library is currently not functional. There was some difficulty in
 * getting the ADC to work properly since it is preconfigured as part of the
 * LoRaWAN stack.
 *
 * Assumes the HAL layer is already initialized. The function, @ref battery_init
 * calibrates the ADC and starts continuous conversion to a variable. The most
 * recent battery voltage level is retrieved with @ref battery_voltage.
 *
 * @{
 */

/**
 * @brief Initializes the battery monitoring module.
 *
 * Starts continuous sampling on ADC.
 *
 * @return Return code
 */
HAL_StatusTypeDef battery_init(void);

/**
 * @brief Gets the most recent battery voltage level
 *
 * The battery voltage level is in mV.
 *
 * @return Battery voltage
 */
unsigned int battery_voltage(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_BATTERY_INCLUDE_BATTERY_H_
