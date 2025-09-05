#include "modules/irrigation.hpp"
#include <ArduinoLog.h>
#include "webserver.hpp"

// Soil moisture thresholds (configurable via web interface)
static float moisture_min_threshold = 50.0f;  // Start irrigation below this %
static float moisture_max_threshold = 75.0f;  // Stop irrigation above this %
static unsigned long check_interval = 900000; // 15 minutes in milliseconds
static unsigned long last_check_time = 0;
bool auto_irrigation_enabled = false;

extern float GetCurrentMoisture();

ModuleIrrigation::ModuleIrrigation(void) {
  // set module type
  type = Esp32Command_irrigation_command_tag;
}

ModuleIrrigation::~ModuleIrrigation(void) {
  // nothing to do
}

void ModuleIrrigation::OnReceive(const Esp32Command &cmd) {
  Log.traceln("ModuleIrrigation::OnReceive");

  if (cmd.which_command != Esp32Command_irrigation_command_tag) {
    return;
  }

  switch (cmd.command.irrigation_command.type) {
    case IrrigationCommand_Type_CHECK:
      Log.traceln("Calling Check");
      Check(cmd);
      break;

    default:
      Log.warningln("Irrigation command type not found!");
      break;
  }
}

size_t ModuleIrrigation::OnRequest(uint8_t *buffer) {
  Log.traceln("ModuleIrrigation::OnRequest");

  // copy request buffer to output buffer
  memcpy(buffer, request_buffer, request_buffer_len);
  return request_buffer_len;
}

void ModuleIrrigation::CheckAutoIrrigation() {
  if (!auto_irrigation_enabled) {
    return;
  }

  unsigned long current_time = millis();
  
  // Check if it's time to monitor
  if (current_time - last_check_time < check_interval) {
    return;
  }
  
  last_check_time = current_time;
  
  // Use the shared irrigation check function
  CheckIrrigationConditions();
}

void ModuleIrrigation::Check(const Esp32Command &cmd) {
  IrrigationCommand resp = IrrigationCommand_init_zero;

  // Check auto-irrigation on each check request
  CheckAutoIrrigation();

  //Get state from webserver
  resp.state = GetSolenoidState();

  Log.noticeln("Responding with state %d", resp.state);

  request_buffer_len =
      EncodeIrrigationCommand(&resp, request_buffer, sizeof(request_buffer));
}

void CheckIrrigationConditions() {
  if (!auto_irrigation_enabled) {
    return;
  }
  
  float current_moisture = GetCurrentMoisture();

  if (current_moisture < 0) {
    Log.errorln("Failed to get soil moisture reading from API");
    return;
  }
  
  Log.noticeln("Irrigation condition check: Moisture = %.1f%% (Thresholds: <%.1f%% open, >%.1f%% close)", 
    current_moisture, moisture_min_threshold, moisture_max_threshold);
  
  // Get current solenoid state
  IrrigationCommand_State current_state = GetSolenoidState();
  
  // Control logic
  if (current_moisture < moisture_min_threshold && current_state != IrrigationCommand_State_OPEN) {
    Log.noticeln("Moisture low (%.1f%% < %.1f%%) - Starting irrigation", 
                 current_moisture, moisture_min_threshold);
    SetSolenoidState(IrrigationCommand_State_OPEN);
  }
  else if (current_moisture > moisture_max_threshold && current_state == IrrigationCommand_State_OPEN) {
    Log.noticeln("Moisture adequate (%.1f%% > %.1f%%) - Stopping irrigation", 
                 current_moisture, moisture_max_threshold);
    SetSolenoidState(IrrigationCommand_State_CLOSE);
  }
}

// Functions to control auto irrigation (called from webserver)
void EnableAutoIrrigation(float min_thresh, float max_thresh) {
  auto_irrigation_enabled = true;
  moisture_min_threshold = min_thresh;
  moisture_max_threshold = max_thresh;
  Log.noticeln("Auto irrigation enabled: Min=%.1f%%, Max=%.1f%%", min_thresh, max_thresh);

  CheckIrrigationConditions();
}

void DisableAutoIrrigation() {
  auto_irrigation_enabled = false;
  Log.noticeln("Auto irrigation disabled");
}

bool IsAutoIrrigationEnabled() {
  return auto_irrigation_enabled;
}

float GetMinThreshold() {
  return moisture_min_threshold;
}

float GetMaxThreshold() {
  return moisture_max_threshold;
}

unsigned long GetCheckInterval() {
  return check_interval;
}

void SetCheckInterval(unsigned long interval_ms) {
  check_interval = interval_ms;
}