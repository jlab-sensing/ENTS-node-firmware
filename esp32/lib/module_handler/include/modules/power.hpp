/**
 * @file power.hpp
 * @author John Madden (jmadden173@pm.me)
 * @brief Module for controlling power on esp32
 * @version 0.1
 * @date 2025-11-05
 *
 */

#ifndef LIB_MODULE_HANDLER_INCLUDE_MODULES_POWER_HPP_
#define LIB_MODULE_HANDLER_INCLUDE_MODULES_POWER_HPP_

#include <ArduinoLog.h>

#include "soil_power_sensor.pb.h"
#include "template_module.hpp"

/**
 * @ingroup moduleHandler
 * @brief Power module for the esp32
 *
 * The power module can put the esp32 in a deep sleep state or wake up the
 * device from deep sleep.
 *
 * @{
 */
    

namespace ModuleHandler {

class ModulePower : public Module {
  public:
    ModulePower();

    ~ModulePower();

    /**
     * @see ModuleHandler::Module.OnReceive
     */
    void OnReceive(const Esp32Command &cmd);

    /**
     * @see ModuleHandler::Module.OnRequest
     */
    size_t OnRequest(uint8_t *buffer);

    /**
     * @brief Put the esp32 into a deep sleep state if stated
     *
     * Wakeup is triggered by an external GPIO pin. Checks if boot_count flag
     * is set internally.
     */
    void EnterSleep();

  private:
    /**
     * @brief Set flag indicating sleep should happen.
     */
    void Sleep();

    /**
     * @brief Returns the reason for the wakeup.
     */
    void Wakeup();

    /** Flag to tell the esp32 to sleep */
    bool sleep_flag = 0;


    /** Pin connected to the wakeup line */
    const gpio_num_t wakeup_pin = GPIO_NUM_3;
  
    /** Buffer for i2c requests */
    uint8_t request_buffer[WiFiCommand_size] = {};
    size_t request_buffer_len = 0;
};

}

/**
 * @}
 */

#endif  // LIB_MODULE_HANDLER_INCLUDE_MODULES_POWER_HPP_
