/**
 * @file page.h
 * @author John Madden (jmadden173@pm.me)
 * @brief Linked list for storing pages of memory
 * @version 0.1
 * @date 2024-08-02
 *
 * @copyright Copyright (c) 2024
 */

#ifndef LIB_STORAGE_INCLUDE_PAGE_H_
#define LIB_STORAGE_INCLUDE_PAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @ingroup storage
 * @defgroup page Page (WIP)
 * @brief Paging system for fram with external sd card (WIP)
 *
 * @verbatim
 * Implements a linked list for storing pages on external memory.
 *
 * Front (Head)                   Back (Tail)
 *     |                              |
 *     v                              v
 * +--------+    +--------+       +--------+
 * | Page 1 | -> | Page 2 | -> ...| Page N |
 * +--------+    +--------+       +--------+
 *     ^                              ^
 *     |                              |
 *    NULL                           NULL
 * @endverbatim
 *
 * Each page in the linked list contains a 'next' pointer pointing towards the
 * tail (Back) and a 'prev' pointer pointing towards the head (Front).
 *
 * @{
 */

typedef struct Page_s Page;

struct Page_s {
  /** Next element in ll */
  Page *next;
  /** Prev element in ll */
  Page *prev;
  /** File index the page corresponds to */
  size_t file_idx;
  /** Flag for file open */
  bool open;
};

/**
 * @brief Loads the last saved page state
 *
 */
void PageInit(void);

/**
 * @brief Safely frees all memory in linked list
 *
 */
void PageDeinit(void);

/**
 * @brief Get the front page
 *
 */
Page *PageFront(void);

/**
 * @brief Get the back page
 *
 */
Page *PageBack(void);

/**
 * @brief Push a page to the front of the linked list
 */
Page *PagePushFront(void);

/**
 * @brief Pop a page from the front of the linked list
 *
 * @param
 */
void PagePopFront(void);

/**
 * @brief Push a page to the back of the linked list
 *
 */
Page *PagePushBack(void);

/**
 * @brief Pop a page from the back of the linked list
 *
 */
void PagePopBack(void);

/**
 * @brief Opens a memory page for read/writing
 *
 * @return Reference to open
 */
void PageOpen(Page *page);

/**
 * @brief Close a page
 *
 * If the page is already closed, nothing is done
 *
 */
void PageClose(Page *page);

/**
 * @brief Writes to a page
 *
 * @param page Page reference
 * @param buf Pointer to buffer
 * @param len Number of bytes to write from buffer
 */
void PageWrite(Page *page, const uint8_t *buf, size_t len);

/**
 * @brief Reads bytes from a page
 *
 * @param page Page reference
 * @param buf Pointer to buffer
 * @param buf_size Max number of bytes to write
 * @return Number of bytes written
 */
size_t PageRead(Page *page, uint8_t *buf, size_t buf_size);

/**
 * @brief Get the number of elements in the linked list
 *
 * @return Number of elements in the linked list
 */
inline size_t PageSize(void);

/**
 * @brief Check if linked list is empty
 *
 * @return true if linked list is empty, false otherwise
 */
inline bool PageEmpty(void);

/**
 * @brief Saves the current page state
 *
 */
void PageStateSave(void);

/**
 * @brief Load the current page state
 *
 */
void PageStateLoad(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif  // LIB_STORAGE_INCLUDE_PAGE_H_
