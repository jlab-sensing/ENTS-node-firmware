#ifndef WIFI_USERCONFIG_HPP
#define WIFI_USERCONFIG_HPP

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

  // Configuration management
  const UserConfiguration *getCurrentConfig() const { return &current_config_; }
  bool hasConfig() const { return has_config_; }
  void updateConfig(const UserConfiguration *pb_config) {
    memcpy(&current_config_, pb_config, sizeof(UserConfiguration));
    has_config_ = true;
    updateWebConfig(pb_config);
    printReceivedConfig();
  }

 private:
  UserConfigCommand current_command_;
  UserConfiguration current_config_;
  bool has_config_ = false;

  void updateWebConfig(const UserConfiguration *pb_config);
  void printReceivedConfig();
};

}  // namespace ModuleHandler

#endif  // WIFI_USERCONFIG_HPP
