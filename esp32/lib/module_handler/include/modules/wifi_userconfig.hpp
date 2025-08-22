#ifndef LIB_USER_CONFIG_INCLUDE_WIFI_USERCONFIG_HPP
#define LIB_USER_CONFIG_INCLUDE_WIFI_USERCONFIG_HPP

#include <ArduinoLog.h>

#include "configuration.hpp"
#include "soil_power_sensor.pb.h"
#include "template_module.hpp"

namespace ModuleHandler {

class ModuleUserConfig : public Module {
 public:
  ModuleUserConfig();
  virtual ~ModuleUserConfig() = default;

  // Implement required Module interface
  void OnReceive(const Esp32Command &cmd) override;
  size_t OnRequest(uint8_t *buffer) override;
 
 private:
  /** @brief Request buffer. */
  uint8_t buffer[Esp32Command_size];
  /** @brief Request buffer len */
  size_t buffer_len = 0;

  /** * @brief Flag to indicate if a configuration has been received
   *
   * This is used to determine if we have a valid configuration to send back
   * when requested.
   */
  bool has_config_ = false;

  /**
   * @brief Update the web server configuration with the received protobuf
   * configuration.
   *
   * This function converts the protobuf configuration to the format used by
   * the web server and updates it accordingly.
   *
   * @param pb_config Pointer to the received user configuration
   */
  void updateWebConfig(const UserConfiguration *pb_config);

  /**
   * @brief Send configuration to the setm32
   *
   * @param cmd The command data.
   *
   */
  void requestConfig(const UserConfigCommand &cmd);

  /**
   * @brief Receive config from the stm32
   *
   * @param cmd The command data.
   */
  void responseConfig(const UserConfigCommand &cmd);
};

}  // namespace ModuleHandler

#endif  // LIB_USER_CONFIG_INCLUDE_WIFI_USERCONFIG_HPP
