// Copyright 2023 jLab, UCSC

/**
 * @file    fifo.c
 * @author  Stephen Taylor
 * @date    11/17/2023
 *
 * @see fifo.h
 **/

#include "fifo.h"

#include "sys_app.h"
#include "usart.h"

static const uint16_t FRAM_BUFFER_READ_ADDR = 0x06F0;   // 1776
static const uint16_t FRAM_BUFFER_WRITE_ADDR = 0x06F2;  // 1778
static const uint16_t FRAM_BUFFER_LEN_ADDR = 0x06F4;    // 1780

// head and tail
static uint16_t read_addr;
static uint16_t write_addr;
static uint16_t buffer_len;

/**
 * @brief Updates circular buffer address based on number of bytes
 *
 * @param addr
 * @param num_bytes
 */
static inline void update_addr(uint16_t *addr, const uint16_t num_bytes) {
  *addr = (*addr + num_bytes) % kFramBufferSize;
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
  } else if (write_addr < read_addr) {
    space_used = kFramBufferSize - (read_addr - write_addr);
  } else {
    // if anything is stored in buffer than entire capacity is used
    // otherwise buffer is empty and all free space is available
    if (buffer_len > 0) {
      space_used = kFramBufferSize;
    } else {
      space_used = 0;
    }
  }

  uint16_t remaining_space = kFramBufferSize - space_used;
  return remaining_space;
}

FramStatus FramPut(const uint8_t *data, const size_t num_bytes) {
  // check remaining space
  if (num_bytes > get_remaining_space()) {
    return FRAM_BUFFER_FULL;
  }

  FramStatus status = FRAM_OK;

  // write single byte length to buffer
  status = FramWrite(write_addr, &num_bytes, 1);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&write_addr, 1);

  // Write data to FRAM circular buffer
  // if the data must wraparound, then make two writes
  if (write_addr + num_bytes > (FRAM_BUFFER_END + 1)) {
    // write up to the buffer end
    size_t num_bytes_first_half = (FRAM_BUFFER_END + 1) - write_addr;
    status = FramWrite(write_addr, data, num_bytes_first_half);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&write_addr, num_bytes_first_half);
    // write from the buffer start
    status = FramWrite(write_addr, data + num_bytes_first_half,
                       num_bytes - num_bytes_first_half);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&write_addr, num_bytes - num_bytes_first_half);
  } else {
    status = FramWrite(write_addr, data, num_bytes);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&write_addr, num_bytes);
  }

  // increment buffer length
  ++buffer_len;

  FramSaveBufferState(read_addr, write_addr, buffer_len);
  return FRAM_OK;
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
  // if the data must wraparound, then make two reads
  if (read_addr + *len > (FRAM_BUFFER_END + 1)) {
    // read up to the buffer end
    size_t len_first_half = (FRAM_BUFFER_END + 1) - read_addr;
    status = FramRead(read_addr, len_first_half, data);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&read_addr, len_first_half);
    // read from the buffer start
    status = FramRead(read_addr, *len - len_first_half, data + len_first_half);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&read_addr, *len - len_first_half);
  } else {
    status = FramRead(read_addr, *len, data);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&read_addr, *len);
  }

  // Decrement buffer length
  --buffer_len;

  FramSaveBufferState(read_addr, write_addr, buffer_len);
  return FRAM_OK;
}

FramStatus FramPeek(size_t idx, uint8_t *data, uint8_t *len) {
  // Check if buffer is empty
  if (buffer_len == 0) {
    return FRAM_BUFFER_EMPTY;
  }
  if (idx >= buffer_len) {
    return FRAM_OUT_OF_RANGE;
  }

  FramStatus status = FRAM_OK;
  uint16_t temp_read_addr = read_addr;

  // advance to idx
  for (size_t i = 0; i < idx; i++) {
    uint8_t temp_len;
    status = FramRead(temp_read_addr, 1, &temp_len);
    if (status != FRAM_OK) {
      return status;
    }
    update_addr(&temp_read_addr, 1 + temp_len);
  }

  // read length
  status = FramRead(temp_read_addr, 1, len);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&temp_read_addr, 1);

  // read data
  status = FramRead(temp_read_addr, *len, data);
  if (status != FRAM_OK) {
    return status;
  }

  return FRAM_OK;
}

FramStatus FramDrop(void) {
  // Check if buffer is empty
  if (buffer_len == 0) {
    return FRAM_BUFFER_EMPTY;
  }

  FramStatus status;
  uint8_t len;

  // read length
  status = FramRead(read_addr, 1, &len);
  if (status != FRAM_OK) {
    return status;
  }
  update_addr(&read_addr, 1 + len);

  // Decrement buffer length
  --buffer_len;

  FramSaveBufferState(read_addr, write_addr, buffer_len);
  return FRAM_OK;
}

uint16_t FramBufferLen(void) { return buffer_len; }

FramStatus FramBufferClear(void) {
  // Set read and write addresses to their default values
  read_addr = FRAM_BUFFER_START;
  write_addr = FRAM_BUFFER_START;

  // reset buffer len
  buffer_len = 0;
  FramSaveBufferState(read_addr, write_addr, buffer_len);

  return FRAM_OK;
}

FramStatus FIFO_Init(void) {
  FramStatus status = FramLoadBufferState(&read_addr, &write_addr, &buffer_len);
  if (status != FRAM_OK) {
    // APP_PRINTF("Failed to load FIFO state. FRAM Status: %d\n", status);
    // If loading the buffer state fails, assume it's an empty state
    // APP_PRINTF("Initialized to empty buffer state.\n");
    return FramBufferClear();
  } else {
    if (read_addr == FRAM_BUFFER_START && write_addr == FRAM_BUFFER_START &&
        buffer_len == 0) {
      APP_PRINTF("Buffer is empty or freshly initialized.\n");
    } else {
      APP_PRINTF("Buffer contains data. Ready to resume operations.\n");
    }
  }
  return FRAM_OK;
}

FramStatus FramSaveBufferState(uint16_t read_addr, uint16_t write_addr,
                               uint16_t buffer_len) {
  uint8_t *read_addr_bytes = (uint8_t *)&read_addr;
  uint8_t *write_addr_bytes = (uint8_t *)&write_addr;
  uint8_t *buffer_len_bytes = (uint8_t *)&buffer_len;

  // Save read_addr
  FramStatus status =
      FramWrite(FRAM_BUFFER_READ_ADDR, read_addr_bytes, sizeof(read_addr));
  if (status != FRAM_OK) {
    // APP_PRINTF("Failed to save read address. FRAM Status: %d\n", status);
    return status;
  }

  // Save write_addr
  status =
      FramWrite(FRAM_BUFFER_WRITE_ADDR, write_addr_bytes, sizeof(write_addr));
  if (status != FRAM_OK) {
    // APP_PRINTF("Failed to save write address. FRAM Status: %d\n", status);
    return status;
  }

  // Save buffer_len
  status =
      FramWrite(FRAM_BUFFER_LEN_ADDR, buffer_len_bytes, sizeof(buffer_len));
  if (status != FRAM_OK) {
    // APP_PRINTF("Failed to save buffer length. FRAM Status: %d\n", status);
    return status;
  }

  // Print a single message summarizing the saved buffer state
  // APP_PRINTF(
  //     "Buffer State Saved Successfully. Read Address: 0x%04X (%d), Write "
  //     "Address: 0x%04X (%d), Buffer Length: %d\n",
  //     read_addr, read_addr, write_addr, write_addr, buffer_len);

  return status;
}

FramStatus FramLoadBufferState(uint16_t *read_addr, uint16_t *write_addr,
                               uint16_t *buffer_len) {
  FramStatus status;

  // Load read_addr and print each byte
  status =
      FramRead(FRAM_BUFFER_READ_ADDR, sizeof(*read_addr), (uint8_t *)read_addr);
  if (status != FRAM_OK) return status;
  // APP_PRINTF("Loaded Read Address: 0x%02X 0x%02X (%d)\n", ((uint8_t
  // *)read_addr)[0],
  //       ((uint8_t *)read_addr)[1], *read_addr);

  // Load write_addr and print each byte
  status = FramRead(FRAM_BUFFER_WRITE_ADDR, sizeof(*write_addr),
                    (uint8_t *)write_addr);
  if (status != FRAM_OK) return status;
  // APP_PRINTF("Loaded Write Address: 0x%02X 0x%02X (%d)\n",
  //       ((uint8_t *)write_addr)[0], ((uint8_t *)write_addr)[1], *write_addr);

  // Load buffer_len and print each byte
  status = FramRead(FRAM_BUFFER_LEN_ADDR, sizeof(*buffer_len),
                    (uint8_t *)buffer_len);
  if (status != FRAM_OK) return status;
  // APP_PRINTF("Loaded Buffer Length: 0x%02X 0x%02X (%d)\n",
  //       ((uint8_t *)buffer_len)[0], ((uint8_t *)buffer_len)[1], *buffer_len);

  return FRAM_OK;
}
