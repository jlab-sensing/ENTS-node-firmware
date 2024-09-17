/**
 * @file wifi.hpp
 * @author John Madden (jmadden173@pm.me)
 * @brief Module for WiFi communication
 * @version 0.1
 * @date 2024-08-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef ESP32_LIB_MODULE_HANDLER_INCLUDE_MODULES_WIFI_H_
#define ESP32_LIB_MODULE_HANDLER_INCLUDE_MODULES_WIFI_H_

#include <WiFi.h>

#include "dirtviz.hpp"

#include "template_module.hpp"

#include "soil_power_sensor.pb.h"
#include "transcoder.h"

class ModuleWiFi: public ModuleHandler::Module {
  public:

  ModuleWiFi(void);

  ~ModuleWiFi(void);

  /**
   * @see ModuleHandler::Module.OnReceive
   */
  void OnReceive(const Esp32Command& cmd);

  /**
   * @see ModuleHandler::Module.OnRequest
   */
  size_t OnRequest(uint8_t* buffer);

  private:

  typedef enum {
    /** Receive WiFi SSID, password, and URL */
    CONNECT = 0,
    /** Post data to the endpoint */
    POST = 1,
  } StateEnum;

  /**
   * @brief Object interface to Dirtviz
   * 
   */
  Dirtviz dirtviz;

  void Post(const Esp32Command& cmd);

  void Connect(const Esp32Command& cmd);

  /** Timeout for wifi connect */
  static const unsigned long connect_timeout_ms = 10000;

  /** Buffer for i2c requests */
  uint8_t* request_buffer[WiFiCommand_size] = {};
  size_t request_buffer_len = 0;
};

#endif // ESP32_LIB_MODULE_HANDLER_INCLUDE_MODULES_WIFI_H_