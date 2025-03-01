/**
 * @file template_module.hpp
 * @author John Madden (jmadden173@pm.me)
 * @brief
 * @version 0.1
 * @date 2024-08-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LIB_MODULE_HANDLER_INCLUDE_TEMPLATE_MODULE_HPP_
#define LIB_MODULE_HANDLER_INCLUDE_TEMPLATE_MODULE_HPP_

#include <cstddef>

#include "soil_power_sensor.pb.h"

namespace ModuleHandler {
class Module {
 public:
  /**
   * @brief Construct a new Module object
   *
   */
  Module(void);

  /**
   * @brief Destroy the Module object
   *
   */
  ~Module(void);

  /**
   * @brief Arduino I2C onRecieve
   *
   * @param num_bytes Number of bytes received from controller
   */
  virtual void OnReceive(const Esp32Command& cmd) = 0;

  /**
   * @brief Arduino I2C onRequest
   *
   * @param buffer Pointer to store requested data
   *
   * @returns Number of bytes in buffer
   */
  virtual size_t OnRequest(uint8_t* buffer) = 0;

  /**
   * @brief Get the current state of the module
   *
   * @return Integer of state enum
   */
  int State(void);

  /**
   * @brief Get the message type associated with the module
   *
   * Full list of message types are available in generated protobuf code
   * under *_tag.
   *
   * @return Integer fo type
   */
  int Type();

  /**
   * @brief Resets the module to default state
   *
   * The default state is
   */
  void Reset(void);

 protected:
  /** Integer representing current state */
  int state = 0;

  /** Integer for message type */
  int type = 0;
};
}  // namespace ModuleHandler

#endif  // LIB_MODULE_HANDLER_INCLUDE_TEMPLATE_MODULE_HPP_
