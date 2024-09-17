#include "module_handler.hpp"

#include <cstring>

#include <Arduino.h>
#include <Wire.h>

#include "transcoder.h"

ModuleHandler::ModuleHandler::ModuleHandler() {}

ModuleHandler::ModuleHandler::~ModuleHandler() {}

void ModuleHandler::ModuleHandler::RegisterModule(int type, Module *module)
{
  // add module to map
  this->req_map[type] = module;
}

void ModuleHandler::ModuleHandler::DeregisterModule(int type) {
  // remove module from map
  this->req_map.erase(type);
}

void ModuleHandler::ModuleHandler::OnReceive(size_t num_bytes) {
  if (this->receive_buffer.len + num_bytes > this->receive_buffer.size) {
    // TODO handle error
  }

  #ifdef UNIT_TEST
  // Store pointer to the rx buffer
  uint8_t* module_handler_rx_buffer_idx = module_handler_rx_buffer;
  // get msg end flag
  bool msg_end = module_handler_rx_buffer_idx;
  // increment idx
  ++module_handler_rx_buffer_idx;
  #else
  // set continue flag to first byte
  bool msg_end = (bool) Wire.read();
  #endif

  // get end of buffer
  uint8_t* prev_end = this->receive_buffer.data + this->receive_buffer.len + 1;

  // loop over available bytes in buffer
  for (uint8_t* end = prev_end; end < prev_end + num_bytes - 1; end++) {
    #ifdef UNIT_TEST
    // copy bytes
    *end = *module_handler_rx_buffer_idx;
    // increment idx
    module_handler_rx_buffer_idx++;
    #else
    // check if available
    if (!Wire.available()) {
      Serial.printf("Specified number of bytes is not available");
    }

    // read data
    *end = Wire.read();
    #endif

    // increment length
    this->receive_buffer.len++;
  }

  // call module OnReceive
  if (msg_end) {
    // decode measurement
    Esp32Command cmd = DecodeEsp32Command(this->receive_buffer.data, this->receive_buffer.len);

    // store reference to module for future OnRequest calls
    this->last_module = this->req_map.at(cmd.which_command);
    // forward command to module
    this->last_module->OnReceive(cmd);
  }
}

void ModuleHandler::ModuleHandler::OnRequest(void) {
  // check if we should send length
  if (this->send_length) {
    // send in little-endian
    uint8_t len_bytes[2] = {};
    len_bytes[1] = (uint8_t) this->request_buffer.len & 0xFF;
    len_bytes[0] = (uint8_t) ((this->request_buffer.len >> 8) & 0xFF);

    // write to buffer
    Wire.write(len_bytes, sizeof(len_bytes));

  // otherwise send data
  } else {
    /** Buffer size for the Wire Arduino library. Determines the max number of
     * bytes that can be communicated. Varies between platforms but avr uses a
     * buffer size of 32. */
    static const int wire_buffer_size = 32;
  
    // only call module if nothing in buffer
    if (this->request_buffer.len == 0) {
      // forward request to last module received
      this->request_buffer.len = this->last_module->OnRequest(this->request_buffer.data);
    }

    // check if length is less than size
    if (this->request_buffer.len > this->request_buffer.size) {
      // TODO handle error
    }

    // optimization if length is less than buffer size
    if (this->request_buffer.len < wire_buffer_size-1) {
      // write finished flag
      Wire.write(1);
      // write data to i2c
      Wire.write(this->request_buffer.data, this->request_buffer.len);
    }

    // get number of bytes remaining
    size_t bytes_remaining = this->request_buffer.len - this->request_buffer.idx;

    // check if length is less than buffer size
    if (bytes_remaining > wire_buffer_size-1) {
      // write finished flag
      Wire.write(1);
      // write directly to i2c
      Wire.write(this->request_buffer.data, this->request_buffer.len);

      // reset to indicate flushed buffer
      this->request_buffer.len = 0;
      this->request_buffer.idx = 0;  
    } else {
      // write unfinished flag
      Wire.write(0);

      // write block of data
      uint8_t* end = this->request_buffer.data + this->request_buffer.idx;
      Wire.write(end, wire_buffer_size-1);

      // increment idx
      this->request_buffer.idx += wire_buffer_size-1;
    }
  }
}

ModuleHandler::Module* ModuleHandler::ModuleHandler::GetModule(int type) {
  return this->req_map.at(type);
}

void ModuleHandler::ModuleHandler::ResetModules(void) {
  // call Reset() for all modules in req_map
  for (auto& entry : this->req_map) {
    entry.second->Reset();
  }
}