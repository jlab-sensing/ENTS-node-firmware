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
 * @brief WiFi module for the esp32

 * The WiFi module support CONNECT and POST commands through OnReceive. The
 * CONNECT command connects to a WiFi network and returns the timestamp from the
 * server for time syncronization purposes. The POST requires sends a HTTP POST
 * to the configured hub URL and returns the data from the HTTP response.
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

  /** Buffer for i2c requests */
  uint8_t request_buffer[Esp32Command_size] = {};
  size_t request_buffer_len = 0;
};

/**
 * @}
 */

#endif  // LIB_MODULE_HANDLER_INCLUDE_MODULES_MICROSD_HPP_
