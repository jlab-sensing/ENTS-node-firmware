#include "controller/controller.h"

#include <stdlib.h>

#include "main.h"
#include "communication.h"
#include "soil_power_sensor.pb.h"

void ControllerInit(void) {
  const size_t buffer_size = Esp32Command_size;

  Buffer *tx = ControllerTx();

  // allocate tx buffer
  tx->data = (uint8_t *)malloc(buffer_size);
  tx->size = buffer_size;
  tx->len = 0;

  Buffer *rx = ControllerRx();

  // allocate rx buffer
  rx->data = (uint8_t *)malloc(buffer_size);
  rx->size = buffer_size;
  rx->len = 0;

  // TODO(jtmaden): Add check for communication
}

void ControllerDeinit(void) {
  Buffer *tx = ControllerTx();

  // free tx buffer
  free(tx->data);
  tx->size = 0;
  tx->len = 0;

  Buffer *rx = ControllerRx();

  // free rx buffer
  free(rx->data);
  rx->size = 0;
  rx->len = 0;
}

void ControllerWakeup(void) {
  HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin,
                        GPIO_PIN_SET);

  HAL_Delay(50);

  HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin,
                        GPIO_PIN_RESET);

  HAL_Delay(50);
}
