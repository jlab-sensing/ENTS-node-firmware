#include "pin.h"

#include "sys_app.h"

// STM32/HAL: GPIO_TypeDef*
// TCA9535: uint8_t
uint32_t port_table[1 + 40] = {
    [3] = (uint32_t)PHYS_PIN_3_PORT,   [5] = (uint32_t)PHYS_PIN_5_PORT,
    [8] = (uint32_t)PHYS_PIN_8_PORT,   [10] = (uint32_t)PHYS_PIN_10_PORT,
    [15] = (uint32_t)PHYS_PIN_15_PORT, [16] = (uint32_t)PHYS_PIN_16_PORT,
    [18] = (uint32_t)PHYS_PIN_18_PORT, [19] = (uint32_t)PHYS_PIN_19_PORT,
    [21] = (uint32_t)PHYS_PIN_21_PORT, [22] = (uint32_t)PHYS_PIN_22_PORT,
    [23] = (uint32_t)PHYS_PIN_23_PORT, [24] = (uint32_t)PHYS_PIN_24_PORT,
    [27] = (uint32_t)PHYS_PIN_27_PORT, [29] = (uint32_t)PHYS_PIN_29_PORT,
    [31] = (uint32_t)PHYS_PIN_31_PORT, [33] = (uint32_t)PHYS_PIN_33_PORT,
    [35] = (uint32_t)PHYS_PIN_35_PORT, [37] = (uint32_t)PHYS_PIN_37_PORT,
    [28] = (uint32_t)PHYS_PIN_28_PORT, [36] = (uint32_t)PHYS_PIN_36_PORT,
};

// STM32/HAL: uint16_t
// TCA9535: uint8_t
uint32_t pin_table[1 + 40] = {
    [3] = PHYS_PIN_3_PIN,   [5] = PHYS_PIN_5_PIN,   [8] = PHYS_PIN_8_PIN,
    [10] = PHYS_PIN_10_PIN, [15] = PHYS_PIN_15_PIN, [16] = PHYS_PIN_16_PIN,
    [18] = PHYS_PIN_18_PIN, [19] = PHYS_PIN_19_PIN, [21] = PHYS_PIN_21_PIN,
    [22] = PHYS_PIN_22_PIN, [23] = PHYS_PIN_23_PIN, [24] = PHYS_PIN_24_PIN,
    [27] = PHYS_PIN_27_PIN, [29] = PHYS_PIN_29_PIN, [31] = PHYS_PIN_31_PIN,
    [33] = PHYS_PIN_33_PIN, [35] = PHYS_PIN_35_PIN, [37] = PHYS_PIN_37_PIN,
    [28] = PHYS_PIN_28_PIN, [36] = PHYS_PIN_36_PIN,
};

PinDevice device_table[1 + 40] = {
    [3] = STM32_GPIO,  [5] = STM32_GPIO,  [8] = STM32_GPIO,  [10] = STM32_GPIO,
    [15] = STM32_GPIO, [16] = STM32_GPIO, [18] = STM32_GPIO, [19] = STM32_GPIO,
    [21] = STM32_GPIO, [22] = STM32_GPIO, [23] = STM32_GPIO, [24] = STM32_GPIO,
    [27] = TCA9535,    [29] = TCA9535,    [31] = TCA9535,    [33] = TCA9535,
    [35] = TCA9535,    [37] = TCA9535,    [28] = TCA9535,    [36] = TCA9535,
};

PinDevice PhysicalPinMap(uint8_t physical_pin, uint32_t* port, uint32_t* pin) {
  PinDevice device = device_table[physical_pin];

  if (device == INVALID) {
    // Some pins (e.g. ground, power) cannot be manipulated.
    APP_LOG(TS_OFF, VLEVEL_H,
            "Pin map: Attempted to access invalid physical pin %u.\r\n",
            physical_pin);
  } else if (device == STM32_GPIO || device == TCA9535) {
    *port = port_table[physical_pin];
    *pin = pin_table[physical_pin];
  }

  return device;
}