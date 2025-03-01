#ifndef LIB_CONTROLLER_SRC_COMMUNICATION_H_
#define LIB_CONTROLLER_SRC_COMMUNICATION_H_

#include <stdint.h>

#include "soil_power_sensor.pb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  /** Success */
  CONTROLLER_SUCCESS = 0,
  /** General error */
  CONTROLLER_ERROR,
  /** Timeout for i2c functions */
  CONTROLLER_TIMEOUT,
  /** For mutex violations */
  CONTROLLER_MUTEX
} ControllerStatus;

typedef struct {
  /** State in buffer */
  uint8_t* data;
  /** Size of buffer */
  size_t size;
  /** Length of data in buffer */
  size_t len;
} Buffer;

/**
 * @brief Send bytes to esp32
 *
 * @param timeout Timeout duration in ms
 */
ControllerStatus ControllerTransmit(unsigned int timeout);

/**
 * @brief Receive bytes from esp32
 *
 * @param timeout Timeout duration in ms
 */
ControllerStatus ControllerReceive(unsigned int timeout);

/**
 * @brief Send message and receive response from esp32
 *
 * @param timeout Timeout duration in ms
 */
ControllerStatus ControllerTransaction(unsigned int timeout);

/**
 * @brief Get reference to transmit buffer
 *
 * Buffers needs to first be initialized with ControllerInit().
 *
 * @return Pointer to transmit buffer
 */
Buffer* ControllerTx(void);

/**
 * @brief Get reference to receive buffer
 *
 * Buffers needs to first be initialized with ControllerInit().
 *
 * @return Pointer to receive buffer
 */
Buffer* ControllerRx(void);

#ifdef __cplusplus
}
#endif

#endif  // LIB_CONTROLLER_SRC_COMMUNICATION_H_
