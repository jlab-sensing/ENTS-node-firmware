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

 
  /**
   * @brief Get the current user configuration.
   *
   * @returns Current user configuration
   */
  const UserConfiguration *getCurrentConfig() const { return &current_config_; }

  /**
   * @brief Check if a configuration has been received.
   *
   * @returns True if a configuration has been received, false otherwise
   */
  bool hasConfig() const { return has_config_; }

  /**
   * @brief Update the current configuration with a new one.
   *
   * This function updates the internal configuration and also updates the web
   * server configuration.
   *
   * @param pb_config Pointer to the new user configuration
   */
  void updateConfig(const UserConfiguration *pb_config) {
    memcpy(&current_config_, pb_config, sizeof(UserConfiguration));
    has_config_ = true;
    updateWebConfig(pb_config);
    printReceivedConfig();
  }

 private:
  UserConfigCommand current_command_;
  UserConfiguration current_config_;

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
};

}  // namespace ModuleHandler

#endif  // LIB_USER_CONFIG_INCLUDE_WIFI_USERCONFIG_HPP
