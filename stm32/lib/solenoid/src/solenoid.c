/**
 ******************************************************************************
 * @file    solenoid.c
 * @author  Caden Jacobs
 *
 * @brief   This library is designed to control an irrigation solenoid
 *          connected to a relay, but can also be used to control other
 *          relay-connected devices.
 * @date    8/6/2025
 ******************************************************************************
 */

#include "solenoid.h"

#include <stdio.h>
#include <stdlib.h>

#include "sys_app.h"  // APP_LOG
#include "tca9535.h"

// This library holds a Solenoid struct for every pin.
// Can be improved / reduced.
static Solenoid s[40];

void SolenoidInit(uint8_t physical_pin, SolenoidType type) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  if (s[physical_pin].type == SOLENOID_TYPE_HOLD_NO) {
    s[physical_pin].state = SOLENOID_STATE_OFF;
  } else if (s[physical_pin].type == SOLENOID_TYPE_HOLD_NC) {
    s[physical_pin].state = SOLENOID_STATE_ON;
  } else if (s[physical_pin].type == SOLENOID_TYPE_LATCH_NO) {
    s[physical_pin].state = SOLENOID_STATE_OFF;
  }

  switch (physical_pin) {
    case 3:
      __HAL_RCC_GPIOA_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_10;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      break;
    case 5:
      __HAL_RCC_GPIOA_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      break;
    case 8:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_6;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 10:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_7;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 15:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 16:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_13;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 18:
      __HAL_RCC_GPIOA_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_15;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      break;
    case 19:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_15;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 21:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_14;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 22:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_3;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 23:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_10;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 24:
      __HAL_RCC_GPIOB_CLK_ENABLE();
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
      GPIO_InitStruct.Pin = GPIO_PIN_4;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      break;
    case 27:  // P10
      TCA9535WritePin(1, 0, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 0, TCA9535_CONFIG_OUTPUT);
      break;
    case 29:  // P11
      TCA9535WritePin(1, 1, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 1, TCA9535_CONFIG_OUTPUT);
      break;
    case 31:  // P12
      TCA9535WritePin(1, 2, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 2, TCA9535_CONFIG_OUTPUT);
      break;
    case 33:  // P13
      TCA9535WritePin(1, 3, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 3, TCA9535_CONFIG_OUTPUT);
      break;
    case 35:  // P14
      TCA9535WritePin(1, 4, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 4, TCA9535_CONFIG_OUTPUT);
      break;
    case 37:  // P15
      TCA9535WritePin(1, 5, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 5, TCA9535_CONFIG_OUTPUT);
      break;
    case 28:  // P16
      TCA9535WritePin(1, 6, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 6, TCA9535_CONFIG_OUTPUT);
      break;
    case 36:  // P17
      TCA9535WritePin(1, 7, GPIO_PIN_RESET);
      TCA9535SetDirection(1, 7, TCA9535_CONFIG_OUTPUT);
      break;
    default:
      // invalid pin
      break;
  }
  s[physical_pin].initialized = true;
}

void SolenoidChangeState(uint8_t physical_pin, SolenoidState state_to_apply) {
  if (state_to_apply == s[physical_pin].state) {
    // Solenoid is already in the correct state.
    return;
  } else if (state_to_apply != SOLENOID_STATE_OFF &&
             state_to_apply != SOLENOID_STATE_ON) {
    // invalid state_to_apply, do not act.
    return;
  }

  uint32_t port;
  uint32_t pin;
  PinDevice dev;
  dev = PhysicalPinMap(physical_pin, &port, &pin);

  if (dev == STM32_GPIO) {
    if (s[physical_pin].type == SOLENOID_TYPE_LATCH_NO) {
      HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin, GPIO_PIN_SET);
      HAL_Delay(SOLENOID_LATCH_DELAY_MS);
      HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin, GPIO_PIN_RESET);
    } else {
      /*
      hold-type vs state_to_apply
      |   +   | NO=0 | NC=1 |
      -------------------------
      | OFF=0 |  0   |  1   |
      | ON =1 |  1   |  2 (% 2 = 0) |
      GPIO_PIN_RESET = 0
      GPIO_PIN_SET = 1
      */
      HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin,
                        (s[physical_pin].type + state_to_apply) % 2);
    }
  } else if (dev == TCA9535) {
    if (s[physical_pin].type == SOLENOID_TYPE_LATCH_NO) {
      TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_SET);
      HAL_Delay(SOLENOID_LATCH_DELAY_MS);
      TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_RESET);
    } else {
      TCA9535WritePin((uint8_t)port, (uint8_t)pin,
                      (s[physical_pin].type + state_to_apply) % 2);
    }
  } else {
    // Invalid pin/device
    return;
  }

  s[physical_pin].state = state_to_apply;
}

void SolenoidChangeStateBatch(uint8_t number_of_pins, uint8_t physical_pin[],
                              SolenoidState state_to_apply[]) {
  uint32_t port;
  uint32_t pin;
  PinDevice dev;
  bool latching_solenoid = false;

  for (int p = 0; p < number_of_pins; p++) {
    if (state_to_apply[p] == s[physical_pin[p]].state) {
      // Solenoid is already in the correct state.
      return;
    } else if (state_to_apply[p] != SOLENOID_STATE_OFF &&
               state_to_apply[p] != SOLENOID_STATE_ON) {
      // invalid state_to_apply, do not act.
      return;
    }

    dev = PhysicalPinMap(physical_pin[p], &port, &pin);

    if (dev == STM32_GPIO) {
      if (s[physical_pin[p]].type == SOLENOID_TYPE_LATCH_NO) {
        HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin, GPIO_PIN_SET);
        // HAL_Delay(SOLENOID_LATCH_DELAY_MS);
        // HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin,
        // GPIO_PIN_RESET);
        // s[physical_pin[p]].state = state_to_apply[p];
        latching_solenoid = true;
      } else if (s[physical_pin[p]].type == SOLENOID_TYPE_HOLD_NO) {
        if (state_to_apply[p] == SOLENOID_STATE_OFF) {
          HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin,
                            GPIO_PIN_RESET);
        } else if (state_to_apply[p] == SOLENOID_STATE_ON) {
          HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin, GPIO_PIN_SET);
        }

        s[physical_pin[p]].state = state_to_apply[p];
      } else if (s[physical_pin[p]].type == SOLENOID_TYPE_HOLD_NC) {
        if (state_to_apply[p] == SOLENOID_STATE_OFF) {
          HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin, GPIO_PIN_SET);
        } else if (state_to_apply[p] == SOLENOID_STATE_ON) {
          HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin,
                            GPIO_PIN_RESET);
        }

        s[physical_pin[p]].state = state_to_apply[p];
      }
    } else if (dev == TCA9535) {
      if (s[physical_pin[p]].type == SOLENOID_TYPE_LATCH_NO) {
        TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_SET);
        // HAL_Delay(SOLENOID_LATCH_DELAY_MS);
        // TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_RESET);
        // s[physical_pin[p]].state = state_to_apply[p];
        latching_solenoid = true;
      } else if (s[physical_pin[p]].type == SOLENOID_TYPE_HOLD_NO) {
        if (state_to_apply[p] == SOLENOID_STATE_OFF) {
          TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_RESET);
        } else if (state_to_apply[p] == SOLENOID_STATE_ON) {
          TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_SET);
        }

        s[physical_pin[p]].state = state_to_apply[p];
      } else if (s[physical_pin[p]].type == SOLENOID_TYPE_HOLD_NC) {
        if (state_to_apply[p] == SOLENOID_STATE_OFF) {
          TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_SET);
        } else if (state_to_apply[p] == SOLENOID_STATE_ON) {
          TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_RESET);
        }

        s[physical_pin[p]].state = state_to_apply[p];
      }

    } else {
      // Invalid pin/device
      return;
    }
  }

  // If there were any latching solenoids, wait for the settle time then perform
  // a second pass through the array to end the control pulse.
  if (latching_solenoid) {
    HAL_Delay(SOLENOID_LATCH_DELAY_MS);
    for (int p = 0; p < number_of_pins; p++) {
      dev = PhysicalPinMap(physical_pin[p], &port, &pin);

      if (dev == STM32_GPIO) {
        if (s[physical_pin[p]].type == SOLENOID_TYPE_LATCH_NO) {
          // HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin,
          // GPIO_PIN_SET); HAL_Delay(SOLENOID_LATCH_DELAY_MS);
          HAL_GPIO_WritePin((GPIO_TypeDef *)port, (uint16_t)pin,
                            GPIO_PIN_RESET);

          s[physical_pin[p]].state = state_to_apply[p];
        }
      } else if (dev == TCA9535) {
        if (s[physical_pin[p]].type == SOLENOID_TYPE_LATCH_NO) {
          // TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_SET);
          // HAL_Delay(SOLENOID_LATCH_DELAY_MS);
          TCA9535WritePin((uint8_t)port, (uint8_t)pin, GPIO_PIN_RESET);

          s[physical_pin[p]].state = state_to_apply[p];
        }

      } else {
        // Invalid pin/device
        return;
      }
    }
  }
}

void SolenoidHandleCommand(LmHandlerAppData_t *appData) {
  if (appData->BufferSize == 0) {
    APP_LOG(TS_OFF, VLEVEL_H, "SolenoidHandleCommand: Buffer empty.\r\n",
            appData->BufferSize);
    return;
  }

  if (appData->Buffer[0] == 1) {
    if (appData->BufferSize != 2) {
      APP_LOG(TS_OFF, VLEVEL_H,
              "SolenoidHandleCommand: V1 command Buffer size invalid (%d "
              "bytes), expected version byte + command byte.\r\n",
              appData->BufferSize);
      return;
    }

    if (appData->Buffer[1] == SOLENOID_COMMAND_CLOSE) {
      for (int i = 0; i < (sizeof(s) / sizeof(Solenoid)); i++) {
        if (s->initialized) {
          SolenoidChangeState(i, SOLENOID_STATE_OFF);
        }
      }
    } else if (appData->Buffer[1] == SOLENOID_COMMAND_OPEN) {
      for (int i = 0; i < (sizeof(s) / sizeof(Solenoid)); i++) {
        if (s->initialized) {
          SolenoidChangeState(i, SOLENOID_STATE_ON);
        }
      }
    } else {
      APP_LOG(TS_OFF, VLEVEL_H,
              "SolenoidHandleCommand: V1 command byte invalid command to apply "
              "%u.\r\n",
              appData->Buffer[1]);
    }
  } else if (appData->Buffer[0] == 2) {
    if ((appData->BufferSize % 2) != 1) {
      APP_LOG(TS_OFF, VLEVEL_H,
              "SolenoidHandleCommand: V2 command buffer size invalid (%d "
              "bytes), expected version byte + 2n bytes.\r\n",
              appData->BufferSize);
      return;
    }

    for (int i = 0; i < appData->BufferSize; i += 2) {
      switch (appData->Buffer[i]) {
        case SOLENOID_COMMAND_CLOSE:
          SolenoidChangeState(appData->Buffer[i + 1], SOLENOID_STATE_OFF);
          break;
        case SOLENOID_COMMAND_OPEN:
          SolenoidChangeState(appData->Buffer[i + 1], SOLENOID_STATE_ON);
          break;
        default:
          APP_LOG(
              TS_OFF, VLEVEL_H,
              "SolenoidHandleCommand: V2 command byte invalid command to apply "
              "%u, skipping.\r\n",
              appData->Buffer[1]);
          break;
      }
    }
  } else {
    APP_LOG(TS_OFF, VLEVEL_H,
            "SolenoidHandlCommand: Version byte invalid %u.\r\n",
            appData->Buffer[0]);
  }
}