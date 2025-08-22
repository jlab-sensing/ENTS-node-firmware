#include "modules/irrigation.hpp"

#include <ArduinoLog.h>
#include "webserver.hpp"

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

void ModuleIrrigation::Check(const Esp32Command &cmd) {
  IrrigationCommand resp = IrrigationCommand_init_zero;

  //Get state from webserver
  resp.state = GetSolenoidState();

  Log.noticeln("Responding with state %d", resp.state);

  request_buffer_len =
      EncodeIrrigationCommand(&resp, request_buffer, sizeof(request_buffer));
}
