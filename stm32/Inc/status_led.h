/** @file status_led.h
 * @brief Header file for controlling the status LED.
 * @author John Madden <jmadden173@pm.me>
 * @date 2025-03-09
 *
 * This file handles the control of the status LED on the Wio-E5 module. It is
 * used to communicate the current state of the device to the user and
 * operates in the following modes:
 *
 *  1. Flashing (Slow)  Device can be configured with the gui
 *  2. Flashing (Fast)  Connecting to the network
 *  3. Off              The device opperating normally and uploading data
 *  4. Solid            An unrecoverable error has occured
 */

#ifndef INC_STATUS_LED_H_
#define INC_STATUS_LED_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the status LED
 *
 * Configured the GPIO port and sets up the necessary timers for the LED. The
 * default state for the LED is off.
 */
void StatusLedInit(void);

/**
 * @brief Flash the status LED slowly
 *
 * The frequency is hardcoded in the source file.
 */
void StatusLedFlashSlow(void);

/**
 * @brief Flash the status LED slowly
 *
 * The frequency is hardcoded in the source file.
 */
void StatusLedFlashFast(void);

/**
 * @brief Turn the status LED off
 */
void StatusLedOff(void);

/**
 * @brief Turn the status LED on
 */
void StatusLedOn(void);

#ifdef __cplusplus
}
#endif

#endif  // INC_STATUS_LED_H_
