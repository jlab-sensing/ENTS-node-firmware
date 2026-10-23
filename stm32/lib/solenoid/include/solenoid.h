/**
 ******************************************************************************
 * @file     solenoid.h
 * @author   Caden Jacobs
 * @brief    This file contains all the function prototypes for
 *           the solenoid.c file.
 *
 *           This library is designed to control an irrigation solenoid
 *           connected to a relay, but can also be used to control other
 *           relay-connected devices.
 * @date     8/6/2025
 ******************************************************************************
 */

#ifndef LIB_SOLENOID_INCLUDE_SOLENOID_H_
#define LIB_SOLENOID_INCLUDE_SOLENOID_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

#include "LmHandler.h"

#ifndef SOLENOID_LATCH_DELAY_MS
// Increase the latch time if the solenoid requires additional time to latch.
#define SOLENOID_LATCH_DELAY_MS 500
#endif

typedef enum {
  SOLENOID_STATE_OFF = 0,
  SOLENOID_STATE_ON = 1,
} SolenoidState;

/*
 Note: The Normally Open (NO) Normally Closed (NC) notation on the SolenoidType
 struct describes circuit energization.

 - A hold-type solenoid that is NO means that it is typically OFF (flow
 blocked), continuously energizing the relay turns it ON (flow permitted).
 - A hold-type solenoid that is NC means that it is typically ON (flow
 permitted), continuously energizing the relay turns it OFF (flow blocked).
 - We always assume that latching solenoids are wired in the NO position. The
 starting state is typically OFF. A pulse will toggle between ON and OFF. A
 latching solenoid retains its state even when deenergized.
*/
typedef enum {
  // standard solenoid, continuously energize to close-circuit/activate.
  SOLENOID_TYPE_HOLD_NO = 0,
  // standard solenoid, continuously energize to open-circuit/deactivate.
  // This option is not typically used.
  SOLENOID_TYPE_HOLD_NC = 1,
  // latching solenoid, pulse to toggle latch. Starts open-circuit/deactivated.
  SOLENOID_TYPE_LATCH_NO,
} SolenoidType;

// state reflects the current state of the solenoid, and should only be modifed
// by the Solenoid* functions (and for initialization).
typedef struct {
  uint32_t pin;  // physical pin # on the 40-pin header.
  SolenoidType type;
  SolenoidState state;  // current state
  bool initialized;
} Solenoid;

// ----------------------------------------------------------------------------

// Downlink messages for solenoid control
// [version_byte] [solenoid_command] [extra_arguments_1] [extra_arguments_2]...

// version_byte = 1
// Apply solenoid_command to ALL initialized solenoid pins.
// Do not specify pins (no extra args).
// Example: [1] [SOLENOID_COMMAND_OPEN]
// ^ opens ALL solenoid pins

// version_byte = 2
// Command:Pin byte pairs specify which command is applied to which pin.
// Must specify pins.
// Example: [2] [SOLENOID_COMMAND_OPEN] [18] [SOLENOID_COMMAND_CLOSE] [19] ...
// ^ open 18, close 19
typedef enum {
  SOLENOID_COMMAND_CLOSE = 0,
  SOLENOID_COMMAND_OPEN,
  // SOLENOID_COMMAND_STATUS_REPORT, // not yet implemented
  // SOLENOID_COMMAND_FORCE_UPDATE_STATE_CLOSE, // not yet implemented
  // SOLENOID_COMMAND_FORCE_UPDATE_STATE_OPEN // not yet implemented
} SolenoidCommand;

// ----------------------------------------------------------------------------

/**
 ******************************************************************************
 * @brief    Wrapper function for the Solenoid initilization.
 *           solenoid->state is reset to the resting position based on type.
 *           i.e. HOLD_NO=OFF, HOLD_NC=ON, LATCH_NO=OFF
 *
 * @note     Do not reinitialize a latching solenoid while it is activated.
 *           This will result in the software state being the inverse of the
 *           physical state of the latching solenoid.
 *
 * @pre      TCA9535Init() if using GPIO expander pins for solenoid control.
 *
 * @param    pin
 * @param    type
 * @return   void
 ******************************************************************************
 */
void SolenoidInit(uint8_t physical_pin, SolenoidType type);

/**
 ******************************************************************************
 * @brief    Changes the state of the solenoid.
 *
 * @note     This function is blocking (SOLENOID_LATCH_DELAY_MS).
 *           Use SolenoidChangeStateBatch() to perform bulk blocking actions.
 *
 * @param    pin
 * @param    state_to_apply
 * @return   void
 ******************************************************************************
 */
void SolenoidChangeState(uint8_t physical_pin, SolenoidState state_to_apply);

/**
 ******************************************************************************
 * @brief    Changes the states of multiple solenoid. All blocking actions
 *           such as from toggling latching solenoids are done in bulk with a
 *           single block.
 *
 * @param    number_of_pins
 * @param    pin
 * @param    state_to_apply
 * @return   void
 ******************************************************************************
 */
void SolenoidChangeStateBatch(uint8_t number_of_pins, uint8_t physical_pin[],
                              SolenoidState state_to_apply[]);

/**
 ******************************************************************************
 * @brief    Function for
 *
 * @param    solenoid
 * @param    state_to_apply
 * @return   void
 ******************************************************************************
 */
void SolenoidHandleCommand(LmHandlerAppData_t* appData);

#ifdef __cplusplus
}
#endif

#endif