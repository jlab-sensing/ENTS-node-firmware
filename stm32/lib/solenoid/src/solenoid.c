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
#include "utilities_def.h"
#include "stm32_seq.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32wlxx_hal_def.h"

static StatusSolenoid Solenoid;
static uint8_t activeSolenoids = 0;
SolenoidParameter *solenoid[SOLENOID_MAX];

void SolenoidInit(SolenoidParameter *solenoid) {
  __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA clock
  __HAL_RCC_GPIOB_CLK_ENABLE();  // Enable GPIOB clock
  
  // Use Pin PA10 to toggle solenoid
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if (solenoid->pin) {
    if (solenoid->pin == 5) {
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);  // LOW = Relay OFF
    }
    if (solenoid->pin == 3) {
      // original solenoid pin used
      GPIO_InitStruct.Pin = GPIO_PIN_10;
      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay OFF
    }
    if (solenoid->pin == 15) {
      GPIO_InitStruct.Pin = GPIO_PIN_9;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);  // LOW = Relay OFF
    }
    if (solenoid->pin == 23) {
      GPIO_InitStruct.Pin = GPIO_PIN_10;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay OFF
    }
    if (solenoid->pin == 19) {
      GPIO_InitStruct.Pin = GPIO_PIN_15;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  // LOW = Relay OFF
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    // Init state of Solenoid
    solenoid->state = SOLENOID_OFF;

  }

}

void SolenoidOpen(SolenoidParameter *solenoid) {
  printf("SOLENOID_OPEN: Setting %i to HIGH (Relay ON)\n", solenoid->pin);
  switch(solenoid->pin)
  {
    case 5:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET);  // HIGH = Relay ON
      solenoid->state = SOLENOID_ON;
      break;

    case 3:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);  // HIGH = Relay ON
      solenoid->state = SOLENOID_ON;
      break;

    case 15:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);  // HIGH = Relay ON
      solenoid->state = SOLENOID_ON;
      break;

    case 23:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);  // HIGH = Relay ON
      solenoid->state = SOLENOID_ON;
      break;

    case 19:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);  // HIGH = Relay ON
      solenoid->state = SOLENOID_ON;
      break;

  }
}

void SolenoidClose(SolenoidParameter *solenoid) {
  printf("SOLENOID_OPEN: Setting %i to LOW (Relay ON)\n", solenoid->pin);
  switch(solenoid->pin)
  {
    case 5:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);  // LOW = Relay OFF
      solenoid->state = SOLENOID_OFF;
      break;

    case 3:
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay OFF
      solenoid->state = SOLENOID_OFF;
      break;

    case 15:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);  // LOW = Relay OFF
      solenoid->state = SOLENOID_OFF;
      break;

    case 23:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay OFF
      solenoid->state = SOLENOID_OFF;
      break;

    case 19:
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  // LOW = Relay OFF
      solenoid->state = SOLENOID_OFF;
      break;

  }
}


/* Private funct*/
// static void SolenoidUpdate(void);

// void SolenoidUpdate(void)
// {
//   // TODO parse the files from downlink.c/h into here, for now it is just a simple toggle
//   if(Solenoid == SOLENOID_OFF)
//   {
//     SolenoidOpen();
//   } else if (Solenoid == SOLENOID_ON)
//   {
//     SolenoidClose();
//   }
// }