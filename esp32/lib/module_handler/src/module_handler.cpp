#include "module_handler.hpp"

#include <Arduino.h>
#include <ArduinoLog.h>
#include <Wire.h>

#include <cstring>

#include "transcoder.h"

ModuleHandler::ModuleHandler::ModuleHandler() {}

ModuleHandler::ModuleHandler::~ModuleHandler() {}

void ModuleHandler::ModuleHandler::RegisterModule(Module *module) {
  // add module to map based on type
  int type = module->Type();
  req_map[type] = module;
}

void ModuleHandler::ModuleHandler::DeregisterModule(int type) {
  // remove module from map
  req_map.erase(type);
}

void ModuleHandler::ModuleHandler::OnReceive(size_t num_bytes) {
  Log.traceln("ModuleHandler::OnReceive");

  if (receive_buffer.len + num_bytes > receive_buffer.size) {
    // TODO(jmadden173) handle error
  }

#ifdef UNIT_TEST
  // Store pointer to the rx buffer
  uint8_t *module_handler_rx_buffer_idx = module_handler_rx_buffer;
  // get msg end flag
  bool msg_end = module_handler_rx_buffer_idx;
  // increment idx
  ++module_handler_rx_buffer_idx;
#else
  // set continue flag to first byte
  bool msg_end = static_cast<bool>(Wire.read());
  Log.verboseln("msg_end: %T", msg_end);
#endif

  // get end of buffer
  uint8_t *prev_end = NULL;
  if (receive_buffer.len == 0) {
    prev_end = receive_buffer.data;
  } else {
    prev_end = receive_buffer.data + receive_buffer.len;
  }

  // loop over available bytes in buffer
  for (uint8_t *end = prev_end; end < prev_end + num_bytes - 1; end++) {
#ifdef UNIT_TEST
    // copy bytes
    *end = *module_handler_rx_buffer_idx;
    // increment idx
    module_handler_rx_buffer_idx++;
#else
    // check if available
    if (!Wire.available()) {
      Log.errorln("Specified number of bytes is not available");
    }

    // read data
    *end = Wire.read();
#endif

    Log.verboseln("Wire.read -> %X", *end);

    // increment length
    receive_buffer.len++;
  }

  // call module OnReceive
  if (msg_end) {
    Log.traceln("Message done");
    Log.traceln("Decoding message");
    for (int i = 0; i < receive_buffer.len; i++) {
      Log.verboseln("receive_buffer[%d] = %X", i, receive_buffer.data[i]);
    }

    // decode measurement
    Esp32Command cmd =
        DecodeEsp32Command(receive_buffer.data, receive_buffer.len);

    Log.verboseln("Forwarding message to: cmd.which_command: %d",
                  cmd.which_command);

    // store reference to module for future OnRequest calls
    if (req_map.find(cmd.which_command) == req_map.end()) {
      Log.errorln("No module registered for command type: %d",
                  cmd.which_command);

      // reset buffer
      receive_buffer.len = 0;
      receive_buffer.idx = 0;
      return;
    }

    last_module = req_map.at(cmd.which_command);

    // forward command to module
    last_module->OnReceive(cmd);

    // reset buffer
    receive_buffer.len = 0;
    receive_buffer.idx = 0;
  }
}

void ModuleHandler::ModuleHandler::OnRequest(void) {
  Log.traceln("ModuleHandler::OnRequest");

  // only call module if nothing in buffer
  if (request_buffer.len == 0) {
    Log.traceln("Copy data into the request buffer");
    // forward request to last module received
    request_buffer.len = last_module->OnRequest(request_buffer.data);
    Log.traceln("Done processing OnRequest");
    Log.traceln("Buffer length: %d", request_buffer.len);
  }

  // check if we should send length
  if (send_length) {
    Log.traceln("Sending length %d", request_buffer.len);

    // send in little-endian
    uint8_t len_bytes[2] = {};
    len_bytes[1] = (uint8_t)request_buffer.len & 0xFF;
    len_bytes[0] = (uint8_t)((request_buffer.len >> 8) & 0xFF);

    Log.verboseln("len_bytes = {%X, %X}", len_bytes[0], len_bytes[1]);

    // write to buffer
    Wire.write(len_bytes, sizeof(len_bytes));

    Log.traceln("Successfully sent length");

    // set flag that length was sent
    send_length = false;

    // otherwise send data
  } else {
    Log.traceln("Sending data");
    /** Buffer size for the Wire Arduino library. Determines the max number of
     * bytes that can be communicated. Varies between platforms but avr uses a
     * buffer size of 32. */
    static const int wire_buffer_size = 32;

    // get number of bytes remaining
    size_t bytes_remaining = request_buffer.len - request_buffer.idx;
    Log.traceln("Bytes remaining: %d", bytes_remaining);

    // print entire buffer
    //for (int i = 0; i < request_buffer.len; i++) {
    //  Log.verboseln("request_buffer[%d] = %X", request_buffer.idx,
    //                request_buffer.data[i]);
    //}

    bool finished = bytes_remaining < wire_buffer_size - 1;
    size_t bytes_to_send = 0;

    // write flag indicating if we are finished
    if (finished) {
      Log.traceln("Writing with finished flag");
      // write finished flag
      Wire.write(1);

      bytes_to_send = bytes_remaining;
    } else {
      Log.traceln("Writing with not finished flag");
      // write unfinished flag
      Wire.write(0);

      bytes_to_send = wire_buffer_size - 1;
    }
    
    // find the end of pr
    uint8_t *end = request_buffer.data + request_buffer.idx;

    // print what will be sent
    for (int i = 0; i < bytes_to_send; i++) {
      Log.verboseln("request_buffer[%d] = %X", request_buffer.idx + i, end[i]);
    }

    Wire.write(end, bytes_to_send);

    request_buffer.idx += bytes_to_send;

    // cleanup after finished
    if (finished) {
      // reset to indicate flushed buffer
      request_buffer.len = 0;
      request_buffer.idx = 0;

      // set flag to send length when complate
      send_length = true;
    }
  }
}

ModuleHandler::Module *ModuleHandler::ModuleHandler::GetModule(int type) {
  return req_map.at(type);
}

void ModuleHandler::ModuleHandler::ResetModules(void) {
  // call Reset() for all modules in req_map
  for (auto &entry : req_map) {
    entry.second->Reset();
  }
}
