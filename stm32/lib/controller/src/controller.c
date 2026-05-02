#include "controller/controller.h"

#include <stdlib.h>

#include "communication.h"
#include "main.h"
#include "soil_power_sensor.pb.h"
#include "tca9535.h"

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

  // Set EN pin for ESP32.
  ControllerDeviceEnable();

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

// Note: Must initialize TCA9535 before calling this function.
void ControllerWakeup(void) {
  TCA9535WritePin(TCA9535_WAKEUP_PORT, TCA9535_WAKEUP_PIN, GPIO_PIN_SET);
  // HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin, GPIO_PIN_SET);

  HAL_Delay(50);

  TCA9535WritePin(TCA9535_WAKEUP_PORT, TCA9535_WAKEUP_PIN, GPIO_PIN_RESET);
  // HAL_GPIO_WritePin(ESP32_WAKEUP_GPIO_Port, ESP32_WAKEUP_Pin,
  // GPIO_PIN_RESET);

  HAL_Delay(50);
}

void ControllerDeviceEnable(void) {
  HAL_GPIO_WritePin(ESP32_EN_GPIO_Port, ESP32_EN_Pin, GPIO_PIN_SET);
}

void ControllerDeviceDisable(void) {
  HAL_GPIO_WritePin(ESP32_EN_GPIO_Port, ESP32_EN_Pin, GPIO_PIN_RESET);
}
