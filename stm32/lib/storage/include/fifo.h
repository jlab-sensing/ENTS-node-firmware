/**
 * @file     fifo.h
 * @author   Stephen Taylor
 * @brief    Circular buffer as FIFO data structure for buffering measurements
 *
 * @date     11/17/2023
 */

#ifndef LIB_STORAGE_INCLUDE_FIFO_H_
#define LIB_STORAGE_INCLUDE_FIFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#include "fram.h"
#include "i2c.h"

/**
 * @ingroup storage
 * @defgroup fifo FIFO
 * @brief Circular buffer for measurements
 *
 * A circular buffer is implemented on part of the memory space of the fram
 * chip. The address space used can be modified with FRAM_BUFFER_START and
 * FRAM_BUFFER_END depending on user needs.
 *
 * Measurements are stored with a single uint8_t of their length followed by
 * the serialized protobuf message. On reads, the length is first read, then
 * the length number of bytes are read into RAM. Ensure the read buffer used
 * is of sufficient size.
 *
 * The buffer's implementation does not allow for overwriting of data. Once
 * the buffer is full, indicated by FRAM_BUFFER_FULL, data needs to be removed
 * by getting the next measurement or clearing the buffer entirely.
 *
 * @todo Implement a clear pointer in the following
 *
 * The buffer operates as a circular queue with three pointers:
 *
 * - Read Pointer: Points to the address of the next measurement to be uploaded.
 * - Write Pointer: Points to the address where the next measurement will be
 * stored.
 * - Clear Pointer: Points to the last measurement that has been confirmed by
 * the backend.
 *
 * Unlike traditional circular buffers that maintain only read and write
 * pointers with overflow protection, the ENTS design adds a clear pointer to
 * track backend uplink confirmations. This pointer ensures that unacknowledged
 * data is not overwritten by subsequent writes. The write pointer is allowed
 * to wrap around but is restricted from advancing past the clear pointer. This
 * constraint protects data integrity in field deployments where network
 * failures may delay acknowledgment indefinitely.
 *
 * @{
 */

#ifndef FRAM_BUFFER_START
/** Starting address of buffer, which is INCLUSIVE */
#define FRAM_BUFFER_START 0
#endif /* FRAM_BUFFER_START */

#ifndef FRAM_BUFFER_END
/** Ending address of buffer, which is INCLUSIVE */
#define FRAM_BUFFER_END 1769
#endif /* FRAM_BUFFER_END */

#if FRAM_BUFFER_START > FRAM_BUFFER_END
#error "Buffer end address must be greater than buffer start address"
#endif

/** Amount of bytes that can be stored in the buffer*/
static const uint16_t kFramBufferSize = FRAM_BUFFER_END - FRAM_BUFFER_START + 1;

/**
 * @brief Puts a measurement into the circular buffer
 *
 * @param    data An array of data bytes.
 * @param    num_bytes The number of bytes to be written.
 * @return   See FramStatus
 */
FramStatus FramPut(const uint8_t *data, uint16_t num_bytes);

/**
 * @brief    Reads a measurement from the queue
 *
 *
 * @param    data Array to be read into
 * @param    len Length of data
 * @return   See FramStatus
 */
FramStatus FramGet(uint8_t *data, uint8_t *len);

/**
 * @brief Get the current number of measurements stored in the buffer
 *
 * @return Number of measurements
 */
uint16_t FramBufferLen(void);

/**
 * @brief Clears the buffer
 *
 * Read and write addresses are set to their default values allowing for the
 * buffer to be overwritten.
 */
FramStatus FramBufferClear(void);

/**
 * @brief Saves the buffer state (read address, write address, and buffer
 * length) to FRAM.
 *
 * @param read_addr Current read address of the circular buffer.
 * @param write_addr Current write address of the circular buffer.
 * @param buffer_len Current length of the circular buffer.
 * @return FramStatus, status of the FRAM operation.
 */
FramStatus FramSaveBufferState(uint16_t read_addr, uint16_t write_addr,
                               uint16_t buffer_len);

/**
 * @brief Loads the buffer state (read address, write address, and buffer
 * length) from FRAM.
 *
 * @param read_addr Pointer to store the retrieved read address.
 * @param write_addr Pointer to store the retrieved write address.
 * @param buffer_len Pointer to store the retrieved buffer length.
 * @return FramStatus, status of the FRAM operation.
 */
FramStatus FramLoadBufferState(uint16_t *read_addr, uint16_t *write_addr,
                               uint16_t *buffer_len);

/**
 * @brief Initializes the FIFO buffer by loading the buffer state (read address,
 *        write address, and buffer length) from FRAM. If the state cannot be
 *        loaded or is invalid, it initializes the buffer with default values.
 * @return FramStatus, status of the FRAM operation.
 */
FramStatus FIFO_Init(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_STORAGE_INCLUDE_FIFO_H_
