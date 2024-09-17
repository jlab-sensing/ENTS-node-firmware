#include "communication.h"

#include <stm32wlxx_hal_i2c.h>
#include "i2c.h"

/**
 * @brief Mutex lock to prevent subsequent transmissions
 * 
 */
static bool g_controller_mutex_lock = false;

/**
 * @brief Buffer size for chunking data
 * 
 * The Arduino Wire libraries (i2c) are limited by a software buffer size of 32
 * by default.
 */
static const int g_i2c_buffer_size = 32;

/**
 * @brief I2C address of the esp32
 * 
 * This is hardcoded on both devices but is variable in case of another device
 * using the same address.
 */
static const uint8_t g_esp32_i2c_addr = 0x20;

/**
 * @brief Wakeup esp32 from deep sleep
 * 
 * Starting with hardware version 2.2.2 there is an stm32 GPIO line connected
 * the esp32 ext1 to wakeup the device from a deep sleep state.
 */
void ControllerWakeupEsp32(void);

/**
 * @brief Converts HAL status to Controller status
 * 
 * @param status HAL status
 * 
 * @return Controller status
 */
ControllerStatus HALToControllerStatus(HAL_StatusTypeDef status);

ControllerStatus ControllerTransmit(unsigned int timeout) {
  // Lock mutex
  g_controller_mutex_lock = true;

  HAL_StatusTypeDef hal_status = HAL_OK;

  // return code storage
  ControllerStatus cont_status = CONTROLLER_SUCCESS;

  // wakeup esp32
  ControllerWakeupEsp32();

  // optimization if length is less than buffer size
  if (tx.len < g_i2c_buffer_size-1) {
    hal_status = HAL_I2C_Master_Transmit(&hi2c2, g_esp32_i2c_addr, tx.data, tx.len, timeout);
    cont_status = HALToControllerStatus(hal_status);
    if (cont_status != CONTROLLER_SUCCESS) {
      return cont_status;
    }
  }

  // create small buffer
  Buffer chunk = {};
  chunk.data = malloc(g_i2c_buffer_size);
  chuck.size = g_i2c_buffer_size;
  chunk.len = 0;

  // shallow copy tx
  Buffer tx_idx = tx;

  // flag for final tx
  bool done = false;


  // handle typical case of data being larger than i2c buffer size
  do {
    // calculate number of bytes to cpy
    size_t num_bytes = 0;
    if (tx_idx.len > (chunk.size-1)) {
      num_bytes = tx_idx.len;
      done = true;
    } else {
      num_bytes = chunk.size-1;
    }

    // first byte is flag
    chunk.data[0] = (uint8_t) done;
    // rest of the data
    memcpy(chunk.data+1, tx_idx.data, num_bytes);
    chunk.len = num_bytes+1;

    // transmit data
    hal_status = HAL_I2C_Master_Transmit(&hi2c2, g_esp32_i2c_addr, chunk.data, chunk.len, timeout); 
    cont_status = HALToControllerStatus(hal_status);
    if (cont_status != CONTROLLER_SUCCESS) {
      break;
    }

    // increment to next block of data
    tx_idx.data += num_bytes;
    tx_idx.len -= num_bytes;
  } while (!done);

  // free allocated memory
  free(chunk.data);

  return cont_status;
}

ControllerStatus ControllerReceive(unsigned int timeout) {
  HAL_StatusTypeDef hal_status = HAL_OK;

  ControllerStatus cont_status = CONTROLLER_SUCCESS;

  // receive number of incoming bytes
  uint8_t len_bytes[2] = {};
  hal_status = HAL_I2C_Master_Receive(&hi2c2, g_esp32_i2c_addr, len_bytes, sizeof(len_bytes), timeout);

  // convert 2 byte array to 16-bit int 
  uint16_t len = (((uint16_t) len_bytes[0]) << 8) | ((uint16_t) len_bytes[1]);

  // set number of bytes
  rx.len = len;

  // optimization if length is less than buffer size
  if (rx.len < g_i2c_buffer_size-1) {
    hal_status = HAL_I2C_Master_Receive(&hi2c2, g_esp32_i2c_addr, rx.data, rx.len, timeout);
    cont_status = HALToControllerStatus(hal_status);
  // otherwise chunk receive
  } else {
    // create small buffer
    Buffer chunk = {};
    chunk.data = malloc(g_i2c_buffer_size);
    chuck.size = g_i2c_buffer_size;
    chunk.len = 0;

    // copy reference to receive
    Buffer rx_idx = rx;

    bool done = false;

    do {
      // calculate number of bytes to receive
      size_t num_bytes = 0;
      if (rx_idx.len > (chunk.size-1)) {
        num_bytes = rx_idx.len;
        done = true;
      } else {
        num_bytes = chunk.size-1;
      }
      // add 1 for done flag
      chunk.len = num_bytes+1;

      // receive block of data
      hal_status = HAL_I2C_Master_Receive(&hi2c2, g_esp32_i2c_addr, chunk.data, chunk.len, timeout);
      cont_status = HALToControllerStatus(hal_status);
      if (cont_status != CONTROLLER_SUCCESS) {
        break;
      }

      // check done flag
      if (done != chunk.data[0]) {
        cont_status = CONTROLLER_ERROR;
        break;
      }

      // copy data to rx
      memcpy(rx_idx.data, chunk.data+1, num_bytes);

      // increment idx
      rx_idx.data += num_bytes;
      rx_idx.len -= num_bytes;
    } while (true);
    
    // free buffer space 
    free(chunk.data);
  }

  // unlock mutex
  g_controller_mutex_lock = false;

  return cont_status;
}

ControllerStatus ControllerTransaction(unsigned int timeout) {
  ControllerTransmit(timeout);
  ControllerReceive(timeout);
}

ControllerStatus HALToControllerStatus(HAL_StatusTypeDef hal_status) {
  if (hal_status == HAL_TIMEOUT) {
    return CONTROLLER_TIMEOUT;
  } else if (hal_status != HAL_OK) {
    return CONTROLLER_ERROR;
  }

  return CONTROLLER_SUCCESS;
}