/**
 ******************************************************************************
 * @file    solenoid.c
 * @author  Caden Jacobs
 *
 * @brief   This library is designed to communicate with a solenoid to open and
 *          close it based on GPIO pins
 *          https://www.orbitonline.com/products/solenoids?variant=45109892612264&country=US&currency=USD&utm_medium=product_sync&utm_source=google&utm_content=sag_organic&utm_campaign=sag_organic&utm_source=paid+search&utm_medium=google+ads&utm_campaign=core+shopping&gad_source=1&gad_campaignid=19652308266&gbraid=0AAAAACw56Spsna6BxGUmTXQEtBiAdD4xG&gclid=CjwKCAjw-svEBhB6EiwAEzSdrsAHw1dSYI86-bRo6kxv42twG7OPn3huLm-9Bx99XAEWvaCfzjyhmxoCmnAQAvD_BwE
 * @date    8/6/2025
 ******************************************************************************
 */

#include "solenoid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32wlxx_hal_def.h"

static StatusSolenoid Solenoid;

void SolenoidInit() {
  __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA clock

  // Use Pin PA10 to toggle solenoid
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Push-pull output
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

  // Set up interrupt
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // Init state of Solenoid
  SolenoidClose();
  Solenoid = SOLENOID_OFF;
}

void SolenoidOpen(void) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay ON
  // Solenoid = SOLENOID_ON;
}

void SolenoidClose(void) {
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);  // HIGH = Relay OFF
  // Solenoid = SOLENOID_OFF;
}
