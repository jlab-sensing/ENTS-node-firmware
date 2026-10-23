// pin.h
// For mapping physical pins to the respective GPIO (or IO Expander) port + pin.
// 2026 10 23
// Jack Lin

#ifndef __PIN_H
#define __PIN_H

#include <stdint.h>

#include "main.h"

typedef enum { INVALID = 0, STM32_GPIO, TCA9535 } PinDevice;

/**
 ******************************************************************************
 * @brief    Provides the port and pin (and device) for the given physical pin.
 *
 * @param    physical_pin 1 - 40
 * @param    port
 * @param    pin
 * @return   PinDevice
 ******************************************************************************
 */
PinDevice PhysicalPinMap(uint8_t physical_pin, uint32_t* port,
                       uint32_t* pin);
#endif