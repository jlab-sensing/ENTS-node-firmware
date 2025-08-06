/**
 ******************************************************************************
* @file    solenoid.c
* @author  Caden Jacobs
*
* @brief   This library is designed to communicate with a solenoid to open and
*          close it based on GPIO pins
* @date    8/6/2025
******************************************************************************
*/
 
#include "solenoid.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "transcoder.h"
#include "stm32wlxx_hal_def.h"

/** States of the solenpid */
typedef enum {
    SOLENOID_OFF,
    SOLENOID_ON,
} StatusSolenoid;

//static StatusSolenoid Solenoid = SOLENOID_OFF;

HAL_StatusTypeDef SolenoidInit() { 

    __HAL_RCC_GPIOA_CLK_ENABLE();  // Enable GPIOA clock
   //Solenoid = SOLENOID_OFF;

    //Use Pin PA10 to toggle solenoid
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // Push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    return ADC_init();
}

void SolenoidOpen(void) {

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);  // LOW = Relay ON
    //Solenoid = SOLENOID_ON;
}

void SolenoidClose(void) {
   
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);  // HIGH = Relay OFF
    //Solenoid = SOLENOID_OFF;
}
