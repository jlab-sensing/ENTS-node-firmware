// Copyright 2023 UCSC

/**
 * @file     fram.h
 * @author   Stephen Taylor
 * @brief    This file contains all the function prototypes for the fram.c file
 *
 * The FM24CL16B memory architecture is structured around 8 segments of 256
 * bytes each. The library can be used with other memory chips with the same
 * memory layout.  The defines FRAM_PAGES and FRAM_SEG_SIZE can used to change
 * the configuration from the compiler.
 *
 * Internally addressing is converted from matrix-style (pages and segments) to
 * a flat memory address space starting at 0x0. The max available memory
 * address is stored in fram_max_addr that can be used for higher level
 * implementations.
 *
 * @date     11/17/2023
 */

#ifndef LIB_STORAGE_SRC_FM24CL16B_H_
#define LIB_STORAGE_SRC_FM24CL16B_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <fram.h>

/** Size of chip in bytes */
static const unsigned int fm24cl16b_size = 2048;

/**
 * @brief Writes bytes to an address
 *
 * @param addr Address of write
 * @param data An array of data bytes.
 * @param len The number of bytes to be written.
 * @return See FramStatus
 */
FramStatus Fm24cl16bWrite(FramAddr addr, const uint8_t *data, size_t len);

/**
 * @brief    This function reads a dynamic number of bytes to FRAM.
 *
 * @param addr Address of read
 * @param data Array to be read into
 * @param len Number of sequential bytes to read
 * @return See FramStatus
 */
FramStatus Fm24cl16bRead(FramAddr addr, size_t len, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif  // LIB_STORAGE_SRC_FM24CL16B_H_
