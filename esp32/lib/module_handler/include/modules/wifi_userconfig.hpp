#ifndef WIFI_USERCONFIG_HPP
#define WIFI_USERCONFIG_HPP

#include "template_module.hpp"
#include "soil_power_sensor.pb.h"

namespace ModuleHandler {

class ModuleUserConfig : public Module {
public:
    ModuleUserConfig();
    virtual ~ModuleUserConfig() = default;

    // Implement required Module interface
    void OnReceive(const Esp32Command &cmd) override;
    size_t OnRequest(uint8_t *buffer) override;
    // Remove Reset() since it's not virtual in base class

private:
    UserConfigCommand current_command_;
    UserConfiguration current_config_;
    bool has_config_ = false;
};

} // namespace ModuleHandler

#endif // WIFI_USERCONFIG_HPP