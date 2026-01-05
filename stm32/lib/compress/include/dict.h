/**
 * @file dict.h
 * @brief Provides abstraction for dictionary operations in compression
 * algorithims
 *
 * Assumes the messages passed have already been serailized via protobuf
 *
 * @author Steve Taylor <stevegtaylor@pm.me>
 * @date 2025-04-01
 */

#ifndef LIB_COMPRESS_INCLUDE_DICT_H
#define LIB_COMPRESS_INCLUDE_DICT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <string.h>

#include "uthash.h"

// Dictionary entry structure
typedef struct {
  char key[256];  // Key (string phrase)
  int value;      // Associated value (index)
  UT_hash_handle hh;
} DictEntry;

// Function declarations
/**
******************************************************************************
* @brief    Init dictionary
*
******************************************************************************
*/
void dict_init();

/**
******************************************************************************
* @brief    Add an entry to dict
*
* @param    const char *key,
* @param    int value
******************************************************************************
*/
void dict_add(const char *key, int value);

/**
******************************************************************************
* @brief    search the dict by key
*
* @param    const char *key,
* @return    int value
******************************************************************************
*/
int dict_search(const char *key);

/**
******************************************************************************
* @brief    free all dict entries
******************************************************************************
*/
void dict_free();

#ifdef __cplusplus
}
#endif

#endif  // LIB_COMPRESS_INCLUDE_DICT_H
