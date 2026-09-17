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

#include "tca9535.h"

// static StatusSolenoid Solenoid;
// static uint8_t activeSolenoids = 0;
// Solenoid *solenoid[SOLENOID_MAX];

void SolenoidInit(Solenoid *solenoid) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  if (solenoid->type == SOLENOID_TYPE_HOLD_NO) {
    solenoid->state = SOLENOID_STATE_OFF;
  } else if (solenoid->type == SOLENOID_TYPE_HOLD_NC) {
    solenoid->state = SOLENOID_STATE_ON;
  } else if (solenoid->type == SOLENOID_TYPE_LATCH_NO) {
    solenoid->state = SOLENOID_STATE_OFF;
  }

  switch (solenoid->pin) {
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
}

void SolenoidChangeState(Solenoid *solenoid, SolenoidState state_to_apply) {
  GPIO_TypeDef *hal_port = GPIOA;
  uint16_t hal_pin = GPIO_PIN_10;
  uint8_t tca9535_port = 1;
  uint8_t tca9535_pin = 0;

  if (state_to_apply == solenoid->state) {
    // Solenoid is already in the correct state.
    return;
  } else if (state_to_apply != SOLENOID_STATE_OFF &&
             state_to_apply != SOLENOID_STATE_ON) {
    // invalid state_to_apply, do not act.
    return;
  }

  switch (solenoid->pin) {
    case 3:
      hal_port = GPIOA;
      hal_pin = GPIO_PIN_10;
      break;
    case 5:
      hal_port = GPIOA;
      hal_pin = GPIO_PIN_9;
      break;
    case 8:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_6;
      break;
    case 10:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_7;
      break;
    case 15:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_9;
      break;
    case 16:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_13;
      break;
    case 18:
      hal_port = GPIOA;
      hal_pin = GPIO_PIN_15;
      break;
    case 19:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_15;
      break;
    case 21:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_14;
      break;
    case 22:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_3;
      break;
    case 23:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_10;
      break;
    case 24:
      hal_port = GPIOB;
      hal_pin = GPIO_PIN_4;
      break;
    case 27:  // P10
      tca9535_port = 1;
      tca9535_pin = 0;
      break;
    case 29:  // P11
      tca9535_port = 1;
      tca9535_pin = 1;
      break;
    case 31:  // P12
      tca9535_port = 1;
      tca9535_pin = 2;
      break;
    case 33:  // P13
      tca9535_port = 1;
      tca9535_pin = 3;
      break;
    case 35:  // P14
      tca9535_port = 1;
      tca9535_pin = 4;
      break;
    case 37:  // P15
      tca9535_port = 1;
      tca9535_pin = 5;
      break;
    case 28:  // P16
      tca9535_port = 1;
      tca9535_pin = 6;
      break;
    case 36:  // P17
      tca9535_port = 1;
      tca9535_pin = 7;
      break;
    default:
      // invalid pin
      return;
      break;
  }

  if (solenoid->pin < 27) {
    // mcu
    if (solenoid->type == SOLENOID_TYPE_LATCH_NO) {
      HAL_GPIO_WritePin(hal_port, hal_pin, GPIO_PIN_SET);
      HAL_Delay(SOLENOID_LATCH_DELAY_MS);
      HAL_GPIO_WritePin(hal_port, hal_pin, GPIO_PIN_RESET);
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
      HAL_GPIO_WritePin(hal_port, hal_pin,
                        (solenoid->type + state_to_apply) % 2);
    }
  } else {
    // tca9535
    if (solenoid->type == SOLENOID_TYPE_LATCH_NO) {
      TCA9535WritePin(tca9535_port, tca9535_pin, GPIO_PIN_SET);
      HAL_Delay(SOLENOID_LATCH_DELAY_MS);
      TCA9535WritePin(tca9535_port, tca9535_pin, GPIO_PIN_RESET);
    } else {
      TCA9535WritePin(tca9535_port, tca9535_pin,
                      (solenoid->type + state_to_apply) % 2);
    }
  }

  solenoid->state = state_to_apply;
}
