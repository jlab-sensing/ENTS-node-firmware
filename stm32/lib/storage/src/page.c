#include "page.h"

#include <stdlib.h>

/**
 * @brief Block size of the i2c communication
 *
 * This can be tuned for performance but is left at 16 to accommodate small
 * hardware buffers.
 */
static const size_t block_size = 16;

static size_t size = 0;

static size_t file_counter = 0;

/** Reference to front of ll */
static Page *front = NULL;

/** Reference to back of ll */
static Page *back = NULL;

Page *AllocatePage(void);

Page *AllocatePage(void) {
  // allocate new page
  Page *new_page = NULL;
  new_page = malloc(sizeof(Page));
  // initial values
  new_page->next = NULL;
  new_page->prev = NULL;
  new_page->open = false;

  return new_page;
}

void PageInit(void) {
  // reset to default values, see variable definitions
  size = 0;
  file_counter = 0;
  front = NULL;
  back = NULL;
}

void PageDeinit(void) {
  // pop all elements from linked list
  while (PageSize() > 0) {
    PagePopFront();
  }
}

Page *PageFront(void) { return front; }

Page *PageBack(void) { return back; }

Page *PagePushFront(void) {
  // allocate new page
  Page *new_page = AllocatePage();

  // check for malloc error
  if (new_page == NULL) {
    return NULL;
  }

  // set file counter
  new_page->file_idx = file_counter;

  // empty linked list
  if (PageEmpty()) {
    front = new_page;
    back = new_page;
  } else {
    new_page->next = front;
    front->prev = new_page;
    front = new_page;
  }

  // update counter
  ++size;
  ++file_counter;

  return new_page;
}

void PagePopFront(void) {
  // do nothing if empty
  if (PageEmpty()) {
    return;
  }

  // close file if open
  if (back->open) {
    PageClose(back);
  }

  // NULL pointers if one element left
  if ((front == back) && (size == 1)) {
    // free memory
    free(front);
    // set null pointers
    front = NULL;
    back = NULL;
    // set next to front
  } else {
    // free memory
    free(front);
    // store tmp pointer to next
    Page *front_next = front->next;
    // "remove" front from linked list
    front_next->prev = NULL;
    // set next as the front
    front = front_next;
  }

  --size;
}

Page *PagePushBack(void) {
  // allocate new page
  Page *new_page = AllocatePage();

  // check for malloc error
  if (new_page == NULL) {
    return NULL;
  }

  // set file counter
  new_page->file_idx = file_counter;

  // case when list is empty
  if (PageEmpty()) {
    front = new_page;
    back = new_page;
    // normal case
  } else {
    new_page->prev = back;
    back->next = new_page;
    back = new_page;
  }

  // update counter
  ++size;
  ++file_counter;

  return new_page;
}

void PagePopBack(void) {
  // do nothing if empty
  if (PageEmpty()) {
    return;
  }

  // close file if open
  if (back->open) {
    PageClose(back);
  }

  // NULL pointers if one element left
  if ((front == back) && (size == 1)) {
    // free memory
    free(back);
    // set null pointers
    front = NULL;
    back = NULL;
    // set next to front
  } else {
    // free memory
    free(back);
    // store tmp pointer to prev
    Page *back_prev = back->prev;
    // "remove" back from linked list
    back_prev->next = NULL;
    // set next as the back
    back = back_prev;
  }

  --size;
}

void PageOpen(Page *page) {
  // TODO(jtmadden): implement

  page->open = true;
}

void PageClose(Page *page) {
  // TODO(jtmadden): implement

  page->open = false;
}

size_t PageSize(void) { return size; }

bool PageEmpty(void) {
  if ((front == NULL) && (back == NULL) && (size == 0)) {
    return true;
  } else {
    return false;
  }
}

void PageStateSave(void) {
  // TODO(jtmadden): implement
}

void PageStateLoad(void) {
  // TODO(jtmadden): implement
}
