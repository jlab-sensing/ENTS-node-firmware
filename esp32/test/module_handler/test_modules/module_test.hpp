#ifndef TEST_MODULE_HANDLER_TEST_MODULES_MODULE_TEST_HPP_
#define TEST_MODULE_HANDLER_TEST_MODULES_MODULE_TEST_HPP_

#include "soil_power_sensor.pb.h"
#include "template_module.hpp"

class ModuleTest : public ModuleHandler::Module {
 public:
  /**
   * @brief Construct a new Module Test object
   *
   */
  ModuleTest(void);

  /**
   * @brief Destroy the Module Test object
   *
   */
  ~ModuleTest(void);

  /** Possible internal states */
  typedef enum {
    INIT = 0,
    RECEIVE = 1,
    RECEIVE_REQUEST = 2,
    REQUEST = 3
  } StateEnum;

  /**
   * @brief Handles i2c receive messages
   *
   * If cmd state is RECEIVE then change internal state
   *
   * If cmd state is REQUEST store inter in data as byte array
   *
   * @param cmd
   */
  void OnReceive(const Esp32Command &cmd);

  /**
   * @brief Handles i2c request messages
   *
   * Replies with data from receive message
   *
   * @param buffer Reference to tx buffer
   * @return Number of bytes written to @p buffer
   */
  size_t OnRequest(uint8_t *buffer);

 private:
  /** Internal buffer for byte array from int32 */
  uint8_t buffer[32] = {};
  /** Length of buffer */
  size_t buffer_len = 0;

  /**
   * @brief Convert an int32 to a byte array
   *
   * @param value int32 value
   * @param buffer Buffer to store byte array
   */
  void Int32ToByteArray(int32_t value, uint8_t *buffer);
};

#endif  // TEST_MODULE_HANDLER_TEST_MODULES_MODULE_TEST_HPP_
