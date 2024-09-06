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
 * Currently user configuration template is provided by this library but will
 * moved in a future version.
 * 
 * @date     11/17/2023 
 */

#ifndef __FRAM_H__
#define __FRAM_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <stdint.h>

#include "i2c.h"

#ifndef FRAM_PAGES
/** Number of pages on the fram chip */
#define FRAM_PAGES 8
#endif

#ifndef FRAM_SEG_SIZE
/** Size of each memory segment in bytes */
#define FRAM_SEG_SIZE 256
#endif

#ifndef BUFFER_STATE_ADDR
/** Address where we will store the buffer state*/
#define BUFFER_STATE_ADDR 1792
#endif

#define USER_DATA_PAGE_ADDRESS 0x07
#define CELL_ID_MEMORY_ADDRESS 0x00
#define LOGGER_ID_MEMORY_ADDRESS 0x08
#define LORA_GATEWAY_EUI_MEMORY_ADDRESS 0x10
#define LORA_APPLICATION_EUI_MEMORY_ADDRESS 0x17
#define LORA_END_DEVICE_EUI_MEMORY_ADDRESS 0x1f
#define LOGGING_INTERVAL_IN_SECONDS_MEMORY_ADDRESS 0x27
#define UPLOAD_INTERVAL_IN_MINUTES_MEMORY_ADDRESS 0x29

typedef struct user_configurations {
  uint64_t cell_ID;
  uint64_t logger_ID;
  uint64_t gateway_EUI;
  uint64_t application_EUI;
  uint64_t end_device_EUI; 
  uint16_t logging_interval; 
  uint16_t upload_interval;
} configuration;

/** Status codes for the Fram library*/
typedef enum {
  FRAM_OK = 0,
  FRAM_ERROR = -1,
  FRAM_OUT_OF_RANGE = -2,
  FRAM_BUFFER_FULL = -3,
  FRAM_BUFFER_EMPTY = -4,
} FramStatus;

/** Max writeable address */
static const uint16_t fram_max_addr = (FRAM_SEG_SIZE * FRAM_PAGES) - 1;

/**
 * @brief Writes bytes to an address
 *
 * 
 *
 * @param addr Address of write
 * @param data An array of data bytes.
 * @param len The number of bytes to be written.
 * @return See FramStatus
 */
FramStatus FramWrite(uint16_t addr, const uint8_t *data, uint8_t len);

/**
 * @brief    This function reads a dynamic number of bytes to FRAM.
 * 
 * @param addr Address of read
 * @param data Array to be read into
 * @param len Number of sequential bytes to read
 * @return See FramStatus
 */
FramStatus FramRead(uint16_t addr, uint8_t len, uint8_t *data);

/**
 * @brief    This function saves buffer state to FRAM.
 * 
 * @param addr Address of store
 * @param data Array to be store into
 * @param len Number of bytes to store
 * @return See FramStatus
 */
FramStatus FramSaveBufferState_Internal(uint16_t read_addr, uint16_t write_addr, uint16_t buffer_len);
FramStatus FramSaveBufferState(void);

/**
 * @brief    This function loads buffer state from FRAM.
 * 
 * @param addr Address to load
 * @param data Array to be load
 * @param len Number of sequential bytes to load
 * @return See FramStatus
 */
FramStatus FramLoadBufferState(uint16_t *read_addr, uint16_t *write_addr, uint16_t *buffer_len);

/**
 * @brief This function stores user configurable settings to non-volatile
 * memory.
 * 
 * Specifically cell ID, logger ID, LoRaWAN gateway EUI, LoRaWAN application
 * EUI and end device EUI. As well as the logging and upload intervals.
 * 
 * @param configuration an instance of the typedef struct user_configurations.
 * Containing all the user defined settings to be stored in non-volatile
 * memory.
 * 
 * @return HAL_StatusTypeDef, status of the I2C function
 */
HAL_StatusTypeDef ConfigureSettings(configuration c);

/**
 * @brief This function reads the user configurable settings from non-volatile
 * memory.
 * 
 * @return configuration, an instance of the typedef struct
 * user_configurations.  Containing all the user defined settings to be stored
 * in non-volatile memory.
 */
configuration ReadSettings(void);

#ifdef __cplusplus
}
#endif

#endif /* __FRAM_H__ */