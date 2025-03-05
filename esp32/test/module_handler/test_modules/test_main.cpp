/**
 * @file test_main.cpp
 * @author John Madden (jmadden173@pm.me)
 * @brief
 * @version 0.1
 * @date 2024-08-12
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <Arduino.h>
#include <unity.h>

#include "module_handler.hpp"
#include "module_test.hpp"

// create handler
static ModuleHandler::ModuleHandler module_handler;
// create test module
static ModuleTest module_test;

void setUp(void) {
  // reset test module
  module_test.Reset();

  // zero out buffers
  memset(module_handler_rx_buffer, 0, module_handler_buffer_size);
  memset(module_handler_tx_buffer, 0, module_handler_buffer_size);
}

void tearDown(void) {}

void TestRegister(void) {
  // register module with message
  module_handler.RegisterModule(2, &module_test);

  bool mod_exist = true;

  // check if module exists in map
  try {
    ModuleHandler::Module* mod = module_handler.GetModule(2);
  } catch (const std::out_of_range& oor) {
    mod_exist = false;
  }

  TEST_ASSERT_TRUE_MESSAGE(mod_exist, "Module does not exist after register");
}

void TestDeregister(void) {
  module_handler.DeregisterModule(2);

  bool mod_exist = true;

  // check that module doesn't exist anymore
  try {
    // get the module of that message type
    ModuleHandler::Module* mod = module_handler.GetModule(2);
  } catch (const std::out_of_range& oor) {
    mod_exist = false;
  }

  TEST_ASSERT_TRUE_MESSAGE(mod_exist, "Module exists after deregister");
}

void TestReset(void) {
  // reset modules
  module_handler.ResetModules();

  // get reference to module
  ModuleHandler::Module* mod = module_handler.GetModule(2);

  // check for the 0th state
  TEST_ASSERT_EQUAL(0, mod->State());
}

void TestOnReceive(void) {
  // definition for test command
  const uint8_t cmd[] = {0x12, 0x2, 0x10, 0x7b};
  const size_t cmd_len = 4;

  // copy command to rx buffer
  memcpy(module_handler_rx_buffer, cmd, cmd_len);

  // simulate i2c receive
  module_handler.OnReceive(cmd_len);

  // check state of module
  ModuleHandler::Module* mod = module_handler.GetModule(2);
  TEST_ASSERT_EQUAL(ModuleTest::RECEIVE, mod->State());

  // check tx buffer has not changed
  TEST_ASSERT_EACH_EQUAL_UINT8(0, module_handler_tx_buffer,
                               module_handler_buffer_size);
}

void TestOnRequest(void) {
  // test command with "request" and data 456
  const uint8_t data[] = {0x12, 0x5, 0x8, 0x2, 0x10, 0xc8, 0x3};
  const size_t data_len = 7;

  // copy data to rx buffer
  memcpy(module_handler_rx_buffer, data, data_len);

  // simulate i2c receive
  module_handler.OnReceive(data_len);

  // check state of module
  ModuleHandler::Module* mod = module_handler.GetModule(2);
  TEST_ASSERT_EQUAL(ModuleTest::RECEIVE, mod->State());

  // check tx buffer has not changed
  TEST_ASSERT_EACH_EQUAL_UINT8(0, module_handler_tx_buffer,
                               module_handler_buffer_size);

  // simulate i2c request
  module_handler.OnRequest();

  // 123 in byte array
  const uint8_t out[] = {0x00, 0x00, 0x00, 0x7B};
  const size_t out_len = 4;

  // compare state
  TEST_ASSERT_EQUAL(ModuleTest::INIT, mod->State());

  // compare to tx buffer
  TEST_ASSERT_EQUAL_HEX8_ARRAY(out, module_handler_tx_buffer, out_len);
}

void setup() {
  // NOTE!!! Wait for >2 secs
  // if board doesn't support software reset via Serial.DTR/RTS
  delay(2000);

  UNITY_BEGIN();

  RUN_TEST(TestRegister);
  RUN_TEST(TestDeregister);
  RUN_TEST(TestReset);

  UNITY_END();
}

void loop() {}
