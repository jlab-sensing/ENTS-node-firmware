/**
 * @file fram.h
 * @author John Madden (jmadden173@pm.me)
 * @brief
 * @version 0.1
 * @date 2024-10-02
 *
 * TODO:
 * - Add function for checking the device ID as a test. Export expected output
 * in each library
 * - Fix multiple pages test to be universal across boards
 *
 * @copyright Copyright (c) 2024
 */

/**
 * @ingroup stm32
 * @defgroup storage Storage
 * @brief Library for storing data on non-volatile memory
 */

#ifndef LIB_STORAGE_INCLUDE_FRAM_H_
#define LIB_STORAGE_INCLUDE_FRAM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "stm32wlxx_hal.h"
#include "sys_app.h"

/**
 * @ingroup storage
 * @defgroup fram FRAM
 * @brief Library for interfacing with the FRAM chip
 *
 * Function calls for read/write ensures data does not exceed size of the
 * buffers.
 *
 * Examples:
 * - @ref example_retrieve_data.c
 *
 * @{
 */

// FRAM Memory Map
#ifndef FRAM_BUFFER_START
/** Starting address of buffer, which is INCLUSIVE */
#define FRAM_BUFFER_START 0
#endif /* FRAM_BUFFER_START */

#ifndef FRAM_BUFFER_END
/** Ending address of buffer, which is INCLUSIVE */
#define FRAM_BUFFER_END 1769
#endif /* FRAM_BUFFER_END */

// FRAM metadata are uint16_t
#define FRAM_BUFFER_READ_ADDR (FRAM_BUFFER_END + 1)
#define FRAM_BUFFER_WRITE_ADDR (FRAM_BUFFER_READ_ADDR + 2)
#define FRAM_BUFFER_LEN_ADDR (FRAM_BUFFER_WRITE_ADDR + 2)

// Address for storing the user config data length in FRAM.
#define USER_CONFIG_LEN_ADDR (FRAM_BUFFER_LEN_ADDR + 2)
// Starting address for user config data in FRAM.
#define USER_CONFIG_START_ADDRESS (USER_CONFIG_LEN_ADDR + 2)

// -----

#define DUMP_FRAM_DISPLAY_HEX 0
#define DUMP_FRAM_DISPLAY_DECIMAL 1
#define DUMP_FRAM_OMIT_NONE 0
#define DUMP_FRAM_OMIT_JUNK 1

/** Status codes for the Fram library*/
typedef enum {
  FRAM_OK = 0,
  FRAM_ERROR = -1,
  FRAM_OUT_OF_RANGE = -2,
  FRAM_BUFFER_FULL = -3,
  FRAM_BUFFER_EMPTY = -4,
} FramStatus;

/** Address size definition */
typedef uint32_t FramAddr;

/**
 * @brief Writes bytes to an address
 *
 * @param addr Address of write
 * @param data An array of data bytes.
 * @param len The number of bytes to be written.
 * @return See FramStatus
 */
FramStatus FramWrite(FramAddr addr, const uint8_t *data, size_t len);

/**
 * @brief    This function reads a dynamic number of bytes to FRAM.
 *
 * @param addr Address of read
 * @param data Array to be read into
 * @param len Number of sequential bytes to read
 * @return See FramStatus
 */
FramStatus FramRead(FramAddr addr, size_t len, uint8_t *data);

/**
 * @brief Get the number of available bytes in FRAM
 *
 * @return Number of bytes
 */
FramAddr FramSize(void);

//
/**
 * @brief This function reads the entirety of non-volatile memory and
 * prints it. Only call FramDump() with input arg linesize=16.
 *
 * @param linesize Number of columns to display. Powers of 2 are recommended.
 * @param displayformat DUMP_FRAM_DISPLAY_HEX or DUMP_FRAM_DISPLAY_DECIMAL
 * @param omitjunk DUMP_FRAM_OMIT_NONE to print all memory in the specified
 * range or DUMP_FRAM_OMIT_JUNK to omit lines that do not hold useful data
 * @param printdelay_ms Delay between printing lines in milliseconds (ms).
 * Recommended value 50 ms.
 * @param startaddress Memory address to start the dump at (inclusive). The
 * linesize aligned memory address will be used.
 * @param endaddress Memory address to end the dump at (inclusive).
 * @return see FramStatus
 */
FramStatus FramDump(uint16_t linesize, uint8_t displayformat, uint8_t omitjunk,
                    uint8_t printdelay_ms, uint16_t startaddress,
                    uint16_t endaddress);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_STORAGE_INCLUDE_FRAM_H_
