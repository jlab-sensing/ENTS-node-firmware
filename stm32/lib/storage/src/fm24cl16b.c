// Copyright 2023 UCSC

/**
 * @file     fram.c
 * @author   Stephen Taylor
 * @brief    This file contains all the function definitions for the fram
 * library.
 *
 * Copyright 2023 jLab, UCSC MIT License
 */
#include "fm24cl16b.h"

#include <stdint.h>

#include "i2c.h"
#include "stm32wlxx_hal_i2c.h"

/** Number of pages on the chip */
static const unsigned int fram_pages = 8;

/** Size of each memory segment in bytes */
static const unsigned int fram_seg_size = 256;

/** Timeout in seconds for the FRAM chip */
static const uint32_t fram_timeout = 100;

/** Device I2C address */
static const uint16_t fram_i2c_addr = 0b10100000;

/** Device address mask for read */
static const uint16_t fram_read_mask = 0b1;

/** Device address mask for write */
static const uint16_t fram_write_mask = 0b0;

/** Matrix representation of memory */
typedef struct {
  /** Page reference */
  uint16_t page;
  /** Segment address */
  uint16_t seg;
} FramAddress;

/**
 * @brief Convert an address to matrix memory representation
 *
 * No out of memory errors are checked.
 *
 * @param addr Memory address
 * @return Matrix representation of address
 */
FramAddress FramConvertAddrMem(FramAddr addr);

FramStatus Fm24cl16bWrite(FramAddr addr, const uint8_t *data, size_t len) {
  // Write byte array to memory
  // NOTE write is performed a single byte at a time due to address
  // configuration of the chip.
  for (uint8_t *d = data; d < data + len; d++) {
    FramAddress addr_mat = FramConvertAddrMem(addr);

    // check for out of memory
    // pages are zero indexed
    if (addr_mat.page > fram_pages - 1) {
      return FRAM_OUT_OF_RANGE;
    }

    // format the i2c address
    uint16_t i2c_addr = fram_i2c_addr | (addr_mat.page << 1) | fram_write_mask;

    // Write byte to an address
    // Can optimize by using continuous writes but needs to account for the
    // change of address.
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Write(&hi2c2, i2c_addr, addr_mat.seg,
                               I2C_MEMADD_SIZE_8BIT, d, 1, fram_timeout);
    // return error if failed to write
    if (status != HAL_OK) {
      return FRAM_ERROR;
    }

    // increment address
    ++addr;
  }

  return FRAM_OK;
}

FramStatus Fm24cl16bRead(FramAddr addr, size_t len, uint8_t *data) {
  // Write byte array to memory
  // NOTE write is performed a single byte at a time due to address
  // configuration of the chip.
  for (uint8_t *d = data; d < data + len; d++) {
    FramAddress addr_mat = FramConvertAddrMem(addr);

    // check for out of memory
    if (addr_mat.page >= fram_pages) {
      return FRAM_OUT_OF_RANGE;
    }
    // format the i2c address
    uint16_t i2c_addr = fram_i2c_addr | (addr_mat.page << 1) | fram_read_mask;

    // Read byte to data address
    HAL_StatusTypeDef status;
    status = HAL_I2C_Mem_Read(&hi2c2, i2c_addr, addr_mat.seg,
                              I2C_MEMADD_SIZE_8BIT, d, 1, fram_timeout);

    // return error if failed to write
    if (status != HAL_OK) {
      return FRAM_ERROR;
    }

    // increment address
    ++addr;
  }

  return FRAM_OK;
}

FramAddress FramConvertAddrMem(FramAddr addr) {
  // convert flat address space to matrix
  FramAddress addr_mat;
  addr_mat.page = addr / fram_seg_size;
  addr_mat.seg = addr % fram_seg_size;

  return addr_mat;
}
