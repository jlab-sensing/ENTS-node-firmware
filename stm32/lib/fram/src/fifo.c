/**
 * @file    fifo.c
 * @author  Stephen Taylor
 * @date    11/17/2023
 * 
 * @see fifo.h
 **/

#include "fifo.h"

uint16_t read_addr = FRAM_BUFFER_START;
uint16_t write_addr = FRAM_BUFFER_START;

/** Number of measurements stored in buffer */
uint16_t buffer_len = 0;

/**
 * @brief Updates circular buffer address based on number of bytes
 * 
 * @param addr 
 * @param num_bytes 
 */
static inline void update_addr(uint16_t *addr, const uint16_t num_bytes) {
  *addr = (*addr + num_bytes) % fram_buffer_size;
}

/**
 * @brief Get the remaining space in the buffer
 * 
 * Calculates the difference between the write and read address in a single
 * forward direction in the circular buffer. If they are equal then nothing has
 * been written.
 * 
 * @return Remaining space in bytes
 */
static uint16_t get_remaining_space(void) {
  uint16_t space_used = 0;
  if (write_addr > read_addr) {
    space_used = write_addr - read_addr;  
  }
  else if (write_addr < read_addr) {
    space_used = fram_buffer_size - (read_addr - write_addr);
  }
  else {
    // if anything is stored in buffer than entire capacity is used
    // otherwise buffer is empty and all free space is available
    if (buffer_len > 0) {
      space_used = fram_buffer_size;
    }
    else {
      space_used = 0;
    }
  }

  uint16_t remaining_space = fram_buffer_size - space_used;
  return remaining_space;
}

FramStatus FramPut(const uint8_t *data, const uint16_t num_bytes) {
  // check remaining space
  if (num_bytes+1 > get_remaining_space()) {
    return FRAM_BUFFER_FULL;
  }

  FramStatus status;

  // write single byte length to buffer
  status = FramWrite(write_addr, &num_bytes, 1);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&write_addr, 1);

  // Write data to FRAM circular buffer
  status = FramWrite(write_addr, data, num_bytes);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&write_addr, num_bytes);

  // increment buffer length
  ++buffer_len;
  return FramSaveBufferState();
}

FramStatus FramGet(uint8_t *data, uint8_t *len) {
  // Check if buffer is empty
  if (buffer_len == 0) {
    return FRAM_BUFFER_EMPTY;
  }

  FramStatus status;

  status = FramRead(read_addr, 1, len);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&read_addr, 1);
  

  // Read data from FRAM circular buffer
  status = FramRead(read_addr, *len, data);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&read_addr, *len);

  // Decrement buffer length
  --buffer_len;
  return FramSaveBufferState();
}

uint16_t FramBufferLen(void) {
  return buffer_len;
}

FramStatus FramBufferClear(void) {
  // Set read and write addresses to their default values
  read_addr = FRAM_BUFFER_START;
  write_addr = FRAM_BUFFER_START;

  // reset buffer len
  buffer_len = 0;
  return FramSaveBufferState();
}

FramStatus FramInitBuffer(void)
{
    FramStatus status = FramLoadBufferState(&read_addr, &write_addr, &buffer_len);
    if (status != FRAM_OK)
    {
        // If loading fails, initialize with default values
        read_addr = FRAM_BUFFER_START;
        write_addr = FRAM_BUFFER_START;
        buffer_len = 0;
        status = FramSaveBufferState();
    }
    return status;
}

FramStatus FramSaveBufferState(void)
{
    return FramSaveBufferState_Internal(read_addr, write_addr, buffer_len);
}

uint16_t FramGetReadAddr(void)
{
    return read_addr;
}

uint16_t FramGetWriteAddr(void)
{
    return write_addr;
}

uint16_t FramGetBufferLen(void)
{
    return buffer_len;
}
