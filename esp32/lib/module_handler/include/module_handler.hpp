/**
 * @file module_handler.hpp
 * @author John Madden (jmadden173@pm.me)
 * @brief Interface for controlling the esp32
 * @version 0.1
 * @date 2024-08-12
 *
 * @copyright Copyright (c) 2024
 *
 */

/**
 * @ingroup esp32
 * @defgroup moduleHandler Module Handler
 * @brief Module handler for the esp32 for communication with the stm32
 */

#ifndef LIB_MODULE_HANDLER_INCLUDE_MODULE_HANDLER_HPP_
#define LIB_MODULE_HANDLER_INCLUDE_MODULE_HANDLER_HPP_

#include <cstddef>
#include <cstdint>
#include <map>

#include "template_module.hpp"
#include "transcoder.h"

// buffers that are used in place of i2c communication
#ifdef UNIT_TEST
static const uint8_t module_handler_buffer_size = 32;
static uint8_t module_handler_rx_buffer[module_handler_buffer_size] = {};
static uint8_t module_handler_tx_buffer[module_handler_buffer_size] = {};
#endif

namespace ModuleHandler {

/**
 * @ingroup moduleHandler
 * @brief Interface for the module handler
 *
 * The @ref ModuleHandler class provides an interface for communication between
 * a host mcu and a client mcu. Commands are encoded via protobuf to save
 * bandwidth on the I2C interface. Every OnReceive call should be followed by an
 * OnRequest call to retrieve status codes or data produced by the command.
 *
 * Functional calls are tied to Arduino Wire library. For unit tests, the define
 * UNIT_TEST creates a global buffer that can be used in place of real i2c
 * communication.
 *
 * @{
 */
class ModuleHandler {
 public:
  /**
   * @brief Construct a new Module Handler object
   *
   */
  ModuleHandler();

  /**
   * @brief Destroy the Module Handler object
   *
   */
  ~ModuleHandler();

  /**
   * @brief Register a module with a message type
   *
   * @param module Pointer to module object
   * @param type Message type
   */
  void RegisterModule(Module *module);

  /**
   * @brief Deregister a module
   *
   * @param type Message type of module
   */
  void DeregisterModule(int type);

  /**
   * @brief Get the module associated with a specific message type
   *
   * @param type Message type
   *
   * @returns Reference to module associated with the type
   */
  Module *GetModule(int type);

  /**
   * @brief Resets all modules
   *
   */
  void ResetModules(void);

  /**
   * @brief Arduino I2C onRecieve
   *
   * @param num_bytes Number of bytes received from controller
   */
  void OnReceive(size_t num_bytes);

  /**
   * @brief Arduino I2C onRequest
   *
   * Due to the I2C buffer size on Arduino, the buffer passed to the module is
   * limited to 32 bytes.
   */
  void OnRequest(void);

 private:
  /** Map of request types to modules */
  std::map<int, Module *> req_map;

  /** Store reference to last module */
  Module *last_module;

  typedef struct {
    /** Size of receive/request buffer */
    const size_t size = Esp32Command_size;
    /** Buffer to store data */
    uint8_t data[Esp32Command_size] = {};
    /** Length of data */
    size_t len = 0;
    /** Current index of buffer */
    size_t idx = 0;
  } Buffer;

  /** Receive buffer */
  Buffer receive_buffer;

  /** Request buffer */
  Buffer request_buffer;

  /** Flag to write length on request before sending data */
  bool send_length = true;
};

/**
 * @}
 */

}  // namespace ModuleHandler

#endif  // LIB_MODULE_HANDLER_INCLUDE_MODULE_HANDLER_HPP_
