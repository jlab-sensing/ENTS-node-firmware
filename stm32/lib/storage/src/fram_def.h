/**
 * @file fram_def.h
 * @author John Madden (jmadden173@pm.me)
 * @brief
 * @version 0.1
 * @date 2024-10-03
 *
 * Fram interface definitions and helper functions. Supported fram chips
 * implement read and write functions and expose @ref FramInterface type. See
 * @ref fm24cl16b.h.
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef LIB_STORAGE_SRC_FRAM_DEF_H_
#define LIB_STORAGE_SRC_FRAM_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "fram.h"

typedef FramStatus (*FramWritePtrType)(FramAddr addr, const uint8_t *data,
                                       size_t len);

typedef FramStatus (*FramReadPtrType)(FramAddr addr, size_t len, uint8_t *data);

typedef struct {
  /** Pointer to write function */
  FramWritePtrType WritePtr;
  /** Pointer to read function */
  FramReadPtrType ReadPtr;
  /** Size of FRAM */
  FramAddr size;
} FramInterfaceType;

/**
 * @brief Converts HAL_StatusTypeDef to FramStatus
 *
 * @param status HAL status code
 * @return Fram status code
 */
inline FramStatus ConvertStatus(HAL_StatusTypeDef status) {
  switch (status) {
    case HAL_OK:
      return FRAM_OK;
    default:
      return FRAM_ERROR;
  }
}

#ifdef __cplusplus
}
#endif

#endif  // LIB_STORAGE_SRC_FRAM_DEF_H_
