#include "unity_config.h"

#include "board.h"

void UnityOutputChar(char c) {
  HAL_StatusTypeDef status;
  status = HAL_UART_Transmit(&huart2, (const uint8_t *)&c, 1, UART_TIMEOUT);
  if (status != HAL_OK) Error_Handler();
}
