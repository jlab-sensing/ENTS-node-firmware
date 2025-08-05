#include "status_led.h"

#include "stm32_seq.h"
#include "stm32_timer.h"
#include "main.h"

const static unsigned int fast_period = 100;  // ms
const static unsigned int slow_period = 500;  // ms

/** States of the status led */
typedef enum {
  LED_OFF,
  LED_ON,
  LED_FLASH
} StatusLedState;

static StatusLedState led_state = LED_OFF;

//
static UTIL_TIMER_Object_t StatusLedTimer;

/**
 * @brief Toggles the status LED state
 */
void StatusLedFlashToggle(void *arg);

void StatusLedInit(void) {
  // set default state to off
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_SET);
  led_state = LED_OFF;

  // set pin to output
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = USER_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USER_LED_GPIO_Port, &GPIO_InitStruct);

  // create the timer for flashing
  UTIL_TIMER_Create(&StatusLedTimer, slow_period, UTIL_TIMER_PERIODIC,
                    StatusLedFlashToggle, NULL);
}

void StatusLedFlashToggle(void *arg) {
  // toggle LED
  HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
}

void StatusLedFlashSlow(void) {
  // set period
  UTIL_TIMER_SetPeriod(&StatusLedTimer, slow_period);
  // start timer
  UTIL_TIMER_Start(&StatusLedTimer);

  led_state = LED_FLASH;
}

void StatusLedFlashFast(void) {
  // set period
  UTIL_TIMER_SetPeriod(&StatusLedTimer, fast_period); 
  // start timer
  UTIL_TIMER_Start(&StatusLedTimer);
  
  led_state = LED_FLASH;
}

void StatusLedOff(void) {
  // stop timer
  if (led_state == LED_FLASH) {
    UTIL_TIMER_Stop(&StatusLedTimer);
  }

  // turn on LED  
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_SET);

  led_state = LED_OFF;
}

void StatusLedOn(void) {
  // stop timer
  if (led_state == LED_FLASH) {
    UTIL_TIMER_Stop(&StatusLedTimer);
  }

  // turn on LED
  HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, GPIO_PIN_RESET);

  led_state = LED_ON;
}
