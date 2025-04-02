/**
 * @file lz78.h
 * @brief Provides declarations for LZ78 serialized message compression.
 *
 * Assumes the messages passed have already been serailized via protobuf
 *
 * @author Steve Taylor <stevegtaylor@pm.me>
 * @date 2025-04-01
 */

#ifndef LIB_COMPRESS_INCLUDE_lz78_H_
#define LIB_COMPRESS_INCLUDE_lz78_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

#include "uthash.h"

/**
******************************************************************************
* @brief    Encode a Protobuf serialized string of bytes using LZ78
*
* @param    const uint8_t *, data
* @param    size_t length
* @return   void
******************************************************************************
*/
void LZ78_encode(const uint8_t *data, size_t length);




#ifdef __cplusplus
}
#endif

#endif  // LIB_COMPRESS_INCLUDE_lz78_H_