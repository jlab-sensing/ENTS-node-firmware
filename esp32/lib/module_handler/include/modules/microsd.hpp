/**
 * @file microsd.hpp
 * @author Jack Lin (jlin143@ucsc.edu)
 * @brief Module for accessing the microSD card
 * @version 0.1
 * @date 2025-07-18
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef LIB_MODULE_HANDLER_INCLUDE_MODULES_MICROSD_HPP_
#define LIB_MODULE_HANDLER_INCLUDE_MODULES_MICROSD_HPP_

#include <Arduino.h>

#include "soil_power_sensor.pb.h"
#include "template_module.hpp"
#include "transcoder.h"

/**
 * @ingroup moduleHandler
 * @brief MicroSD module for the esp32

 * The microSD module supports Save commands through OnReceive. The
 * Save command decodes protobuf-serialized data and stores it into a CSV file on the microSD card.
 *
 * @{
 */

class ModuleMicroSD : public ModuleHandler::Module {
 public:
  ModuleMicroSD(void);

  ~ModuleMicroSD(void);

  /**
   * @see ModuleHandler::Module.OnReceive
   */
  void OnReceive(const Esp32Command &cmd);

  /**
   * @see ModuleHandler::Module.OnRequest
   */
  size_t OnRequest(uint8_t *buffer);

 private:

  void Save(const Esp32Command &cmd);
  void Time(const Esp32Command &cmd);
  void Size(const Esp32Command &cmd);

  /** Buffer for i2c requests */
  uint8_t request_buffer[Esp32Command_size] = {};
  size_t request_buffer_len = 0;
};

/**
 * @}
 */

#endif  // LIB_MODULE_HANDLER_INCLUDE_MODULES_MICROSD_HPP_
