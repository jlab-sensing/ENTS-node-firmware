/**
 * @file irrigation.hpp
 * @author John Madden
 * @brief Module for controling irrigation valuve
 * @date 2025-08-19
 *
 *
 */

#ifndef LIB_MODULE_HANDLER_INCLUDE_MODULES_IRRIGATION_HPP_
#define LIB_MODULE_HANDLER_INCLUDE_MODULES_IRRIGATION_HPP_

#include <Arduino.h>

#include "transcoder.h"
#include "template_module.hpp"
#include "webserver.hpp"

/**
 * @ingroup moduleHandler
 * @brief Irrigation module for the esp32
 *
 * The irrigation module supports opening and closing an irrigation value that
 * is connect to the esp32.
 *
 * @{
 */

class ModuleIrrigation : public ModuleHandler::Module {
 public:
  ModuleIrrigation(void);

  ~ModuleIrrigation(void);

  /**
   * @see ModuleHandler::Module.OnReceive
   */
  void OnReceive(const Esp32Command &cmd);

  /**
   * @see ModuleHandler::Module.OnRequest
   */
  size_t OnRequest(uint8_t *buffer);

 private:
  /**
   * @brief Get the current state and send back to stm32.
   */
  void Check(const Esp32Command &cmd);

  /** Buffer for i2c requests */
  uint8_t request_buffer[WiFiCommand_size] = {};
  size_t request_buffer_len = 0;
};

/**
 * @}
 */

#endif  // LIB_MODULE_HANDLER_INCLUDE_MODULES_IRRIGATION_HPP_
