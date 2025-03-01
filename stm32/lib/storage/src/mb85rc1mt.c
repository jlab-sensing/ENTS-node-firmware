#include "mb85rc1mt.h"

#include <string.h>

#include "fram_def.h"
#include "i2c.h"
#include "stm32wlxx_ll_i2c.h"

/** Base address of chip */
static const uint8_t g_base_addr = 0b10100000;

/** Timeout for i2c communication. Set to greater than wakeup time */
static const uint32_t g_timeout = 1000;

/** Number of pages on the chip */
static const int mb85rc1mt_pages = 1;

/** Size of each memory segment in bytes */
static const int mb85rc1mt_seg_size = 1 << 17;

/** Representation of memory address */
typedef struct {
  /** Device address */
  uint8_t dev;
  /** Memory address */
  uint16_t mem;
} Mb85rc1mtAddress;

/**
 * @brief Put the chip into sleep mode
 *
 */
FramStatus Sleep(Mb85rc1mtAddress addr);

/**
 * @brief Wake up device from sleep
 *
 * @return
 */
FramStatus Wakeup(Mb85rc1mtAddress addr);

/**
 * @brief Convert flat fram address to mb85rc1mt formatted address
 *
 * @param addr Fram address
 * @return Mb85rc1mt address
 */
Mb85rc1mtAddress ConvertAddress(FramAddr addr);

FramStatus Mb85rc1mtWrite(FramAddr addr, const uint8_t *data, size_t len) {
  HAL_StatusTypeDef hal_status = HAL_OK;
  FramStatus fram_status = FRAM_OK;

  while (len > 0) {
    // convert fram address to i2c address
    Mb85rc1mtAddress i2c_addr = ConvertAddress(addr);

    // wakeup device
    fram_status = Wakeup(i2c_addr);
    if (fram_status != FRAM_OK) {
      return fram_status;
    }

    // number of bytes that can be written without changing address
    size_t write_len = 0;
    if ((size_t)addr + len > mb85rc1mt_seg_size) {
      write_len = mb85rc1mt_seg_size - addr;
    } else {
      write_len = len;
    }

    for (int i = 0; i < write_len; i++) {
      // transmit data
      hal_status =
          HAL_I2C_Mem_Write(&hi2c2, i2c_addr.dev, i2c_addr.mem,
                            I2C_MEMADD_SIZE_16BIT, data, len, g_timeout);
      fram_status = ConvertStatus(hal_status);
      if (fram_status != FRAM_OK) {
        return fram_status;
      }
    }

    // update address and length
    addr += write_len;
    len -= write_len;

    // sleep device
    fram_status = Sleep(i2c_addr);
    if (fram_status != FRAM_OK) {
      return fram_status;
    }
  }

  // return status
  return fram_status;
}

FramStatus Mb85rc1mtRead(FramAddr addr, size_t len, uint8_t *data) {
  FramStatus fram_status = FRAM_OK;

  while (len > 0) {
    // convert fram address to i2c address
    Mb85rc1mtAddress i2c_addr = ConvertAddress(addr);

    // wakeup device
    fram_status = Wakeup(i2c_addr);
    if (fram_status != FRAM_OK) {
      return fram_status;
    }

    size_t read_len = 0;
    if ((size_t)addr + len > mb85rc1mt_seg_size) {
      read_len = mb85rc1mt_seg_size - addr;
    } else {
      read_len = len;
    }

    // transmit data
    HAL_StatusTypeDef hal_status = HAL_OK;

    // read from memory
    hal_status = HAL_I2C_Mem_Read(&hi2c2, i2c_addr.dev | 1, i2c_addr.mem,
                                  I2C_MEMADD_SIZE_16BIT, data, len, g_timeout);
    fram_status = ConvertStatus(hal_status);
    if (fram_status != FRAM_OK) {
      return fram_status;
    }

    // update addr and len
    addr += read_len;
    len -= read_len;

    // sleep device
    fram_status = Sleep(i2c_addr);
    if (fram_status != FRAM_OK) {
      return fram_status;
    }
  }

  // return status
  return fram_status;
}

FramStatus Sleep(Mb85rc1mtAddress addr) {
  /*
  // address to enter sleep
  const uint8_t sleep_addr = 0xf8;

  // data sent to enter sleep
  const uint8_t sleep_cmd = 0x86;

  size_t buffer_len = 2;
  uint8_t buffer[buffer_len];

  buffer[0] = addr.dev;
  buffer[1] = sleep_cmd;

  HAL_StatusTypeDef status = HAL_OK;
  status = HAL_I2C_Master_Transmit(&hi2c2, sleep_addr, buffer, buffer_len,
                                   g_timeout);

  return ConvertStatus(status);
  */
  return FRAM_OK;
}

FramStatus Wakeup(Mb85rc1mtAddress addr) {
  /*
  HAL_StatusTypeDef status = HAL_OK;
  status = HAL_I2C_Master_Transmit(&hi2c2, addr.dev, NULL, 0, 0);
  */
  /*
  // expect an error
  if (status != HAL_ERROR) {
    return FRAM_ERROR;
  }
  */

  return FRAM_OK;
}

Mb85rc1mtAddress ConvertAddress(FramAddr addr) {
  Mb85rc1mtAddress new_addr = {};

  // max address bytes in device address.
  // shift in 15 is to account to remove lower 16 bytes and have correct
  // position in the address before the r/w bit.
  new_addr.dev = g_base_addr | ((addr >> 15) & 0b1110);
  new_addr.mem = addr & 0xFFFF;

  return new_addr;
}
