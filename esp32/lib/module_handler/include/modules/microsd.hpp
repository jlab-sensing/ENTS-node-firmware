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
#include "sensor.pb.h"
#include "template_module.hpp"
#include "transcoder.h"

/**
 * @ingroup moduleHandler
 * @brief MicroSD module for the esp32

 * The microSD module supports Save commands through OnReceive. The
 * Save command decodes protobuf-serialized data and stores it into a CSV file
 on the microSD card.
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
  /**
   * @brief Saves the measurement as a CSV (or raw data) to the micro SD card.
   *
   * The `which_data` field identifies the data field as either an encoded
   * measurement or raw bytes.
   *
   * If saving an encoded measurement, the provided filename will be used.
   * Otherwise (null string filename), a filename will be generated based on the
   * measurement type (Ex. "/1_POWER_VOLTAGE.csv") and the measurement data and
   * metadata is appended to the end of the CSV.
   *
   * For measurements, the first row of the CSV is the header row.
   *
   * unix_time | cell_id | logger_id | sensor_data
   *    ...    |   ...   |    ...    |     ...
   *
   * If saving raw bytes, the provided filename will be used.
   * Otherwise (null string filename), the filename "/raw.csv" will be used and
   * data will be appended to the end of that file.
   * 
   * @todo Support RepeatedSensorMeasurements
   */
  void Save(const Esp32Command &cmd);
  /**
   * @brief Save the User Configuration to a plain text file on the micro SD
   * card.
   */
  void UserConfig(const Esp32Command &cmd);

  /** Buffer for i2c requests */
  uint8_t request_buffer[MicroSDCommand_size] = {};
  size_t request_buffer_len = 0;
};

/**
 * @}
 */

#endif  // LIB_MODULE_HANDLER_INCLUDE_MODULES_MICROSD_HPP_
