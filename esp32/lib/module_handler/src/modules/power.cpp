#include "modules/power.hpp"

#include <ArduinoLog.h>

#include "transcoder.h"
#include "driver/gpio.h"
#include "esp_sleep.h"

namespace ModuleHandler {

  /** Boot count */
RTC_DATA_ATTR static unsigned int boot_count = 0;

ModulePower::ModulePower() {
  // set module type
  type = Esp32Command_power_command_tag;

  ++boot_count;
}

ModulePower::~ModulePower() {}

void ModulePower::OnReceive(const Esp32Command &cmd) {
  Log.traceln("ModulePower::OnReceive");

  // check if power command
  if (cmd.which_command != Esp32Command_power_command_tag) {
    return;
  }

  Log.traceln("PowerCommand type: %d", cmd.command.power_command.type);

  // switch for command types
  switch (cmd.command.power_command.type) {
    case PowerCommand_Type_SLEEP:
      Log.traceln("Calling SLEEP");
      Sleep();
      break;

    case PowerCommand_Type_WAKEUP:
      Log.traceln("Calling REASON");
      Wakeup();
      break;

    default:
      Log.warningln("Power command type not found!");
      break;
  }
}

size_t ModulePower::OnRequest(uint8_t *buffer) {
  Log.traceln("ModulePower::OnRequest");
  memcpy(buffer, request_buffer, request_buffer_len);
  return request_buffer_len;
}

void ModulePower::Sleep() {
  Log.noticeln("Setting sleep flag");
  sleep_flag = true;
 
  PowerCommand resp = PowerCommand_init_zero;
  resp.type = PowerCommand_Type_SLEEP;

  request_buffer_len =
      EncodePowerCommand(&resp, request_buffer, sizeof(request_buffer));
}

void ModulePower::Wakeup() {
  Log.noticeln("Wakeup called");

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

  PowerCommand resp = PowerCommand_init_zero;
  resp.type = PowerCommand_Type_WAKEUP;
  resp.reason = static_cast<PowerCommand_WakeupReason>(wakeup_reason);
  resp.boot_count = boot_count;

  request_buffer_len =
      EncodePowerCommand(&resp, request_buffer, sizeof(request_buffer));
}

void ModulePower::EnterSleep() {
  // only enter sleep if flag is set
  if (!sleep_flag) {
    return;
  }

  Log.noticeln("Entering deep sleep mode");

  // configure gpio pin
  const gpio_config_t config = {
      .pin_bit_mask = BIT(GPIO_NUM_3),
      .mode         = GPIO_MODE_INPUT,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
  };
  gpio_config(&config);
  esp_deep_sleep_enable_gpio_wakeup(BIT(wakeup_pin), ESP_GPIO_WAKEUP_GPIO_HIGH);

  // enter deep sleep
  esp_deep_sleep_start();
}

}  // namespace ModuleHandler
